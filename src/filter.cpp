#include "include\filter.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QColor>

// --- None Filter ---
class NoneFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::None; }
    std::vector<FilterParamInfo> getParamInfo() const override { return {}; }
    QImage apply(const QImage& input, const std::vector<float>&) override { return input; }
};

// --- Grayscale Filter ---
class GrayscaleFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::Grayscale; }
    std::vector<FilterParamInfo> getParamInfo() const override { return {}; }
    QImage apply(const QImage& input, const std::vector<float>&) override {
        QImage res = input;
        for (int y = 0; y < res.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(res.scanLine(y));
            for (int x = 0; x < res.width(); ++x) {
                int g = qGray(line[x]);
                line[x] = qRgba(g, g, g, qAlpha(line[x]));
            }
        }
        return res;
    }
};

// --- Invert Filter ---
class InvertFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::Invert; }
    std::vector<FilterParamInfo> getParamInfo() const override { return {}; }
    QImage apply(const QImage& input, const std::vector<float>&) override {
        QImage res = input;
        res.invertPixels(QImage::InvertRgb);
        return res;
    }
};

// --- Brightness / Contrast Filter ---
class BrightnessContrastFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::BrightnessContrast; }
    std::vector<FilterParamInfo> getParamInfo() const override {
        return { {"Brightness", -100.0f, 100.0f, 0.0f}, {"Contrast", -100.0f, 100.0f, 0.0f} };
    }
    QImage apply(const QImage& input, const std::vector<float>& params) override {
        if (params.size() < 2) return input;
        float b = params[0], c = params[1];
        if (b == 0.0f && c == 0.0f) return input;

        QImage res = input;
        float factor = (259.0f * (c + 255.0f)) / (255.0f * (259.0f - c));
        for (int y = 0; y < res.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(res.scanLine(y));
            for (int x = 0; x < res.width(); ++x) {
                QRgb p = line[x];
                if (qAlpha(p) == 0) continue;
                int r = qBound(0, (int)(factor * (qRed(p) - 128) + 128 + b), 255);
                int g = qBound(0, (int)(factor * (qGreen(p) - 128) + 128 + b), 255);
                int bl = qBound(0, (int)(factor * (qBlue(p) - 128) + 128 + b), 255);
                line[x] = qRgba(r, g, bl, qAlpha(p));
            }
        }
        return res;
    }
};

// --- Blur Filter ---
class BlurFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::Blur; }
    std::vector<FilterParamInfo> getParamInfo() const override { return { {"Radius", 0.0f, 50.0f, 0.0f} }; }
    QImage apply(const QImage& input, const std::vector<float>& params) override {
        if (params.empty() || params[0] <= 0.0f) return input;
        QImage res(input.size(), QImage::Format_ARGB32_Premultiplied);
        res.fill(Qt::transparent);

        QGraphicsScene scene;
        QGraphicsPixmapItem* item = scene.addPixmap(QPixmap::fromImage(input));
        QGraphicsBlurEffect* eff = new QGraphicsBlurEffect();
        eff->setBlurRadius(params[0]);
        item->setGraphicsEffect(eff);

        QPainter p(&res);
        scene.render(&p);
        return res;
    }
};

// --- Threshold Filter ---
class ThresholdFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::Threshold; }
    std::vector<FilterParamInfo> getParamInfo() const override { return { {"Level", 0.0f, 255.0f, 128.0f} }; }
    QImage apply(const QImage& input, const std::vector<float>& params) override {
        if (params.empty()) return input;
        int threshold = static_cast<int>(params[0]);
        QImage res = input;
        for (int y = 0; y < res.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(res.scanLine(y));
            for (int x = 0; x < res.width(); ++x) {
                if (qAlpha(line[x]) == 0) continue;
                int g = qGray(line[x]);
                int val = g >= threshold ? 255 : 0;
                line[x] = qRgba(val, val, val, qAlpha(line[x]));
            }
        }
        return res;
    }
};

// --- HSL Filter ---
class HSLFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::HSL; }
    std::vector<FilterParamInfo> getParamInfo() const override {
        return { {"Hue Shift", -180.0f, 180.0f, 0.0f}, {"Saturation", -100.0f, 100.0f, 0.0f}, {"Lightness", -100.0f, 100.0f, 0.0f} };
    }
    QImage apply(const QImage& input, const std::vector<float>& params) override {
        if (params.size() < 3) return input;
        int h_shift = static_cast<int>(params[0]);
        int s_shift = static_cast<int>(params[1]);
        int l_shift = static_cast<int>(params[2]);

        if (h_shift == 0 && s_shift == 0 && l_shift == 0) return input;

        QImage res = input;
        for (int y = 0; y < res.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(res.scanLine(y));
            for (int x = 0; x < res.width(); ++x) {
                if (qAlpha(line[x]) == 0) continue;
                QColor c(line[x]);
                int h, s, l, a;
                c.getHsl(&h, &s, &l, &a);

                if (h != -1) h = (h + h_shift + 360) % 360;
                s = qBound(0, s + (s_shift * 255 / 100), 255);
                l = qBound(0, l + (l_shift * 255 / 100), 255);

                c.setHsl(h == -1 ? 0 : h, s, l, a);
                line[x] = c.rgba();
            }
        }
        return res;
    }
};

// --- Sharpen Filter ---
class SharpenFilter : public BaseFilter {
public:
    FilterType getType() const override { return FilterType::Sharpen; }
    std::vector<FilterParamInfo> getParamInfo() const override { return { {"Strength", 0.0f, 10.0f, 0.0f} }; }
    QImage apply(const QImage& input, const std::vector<float>& params) override {
        if (params.empty() || params[0] <= 0.0f) return input;
        float strength = params[0] / 2.0f; // Уменьшим агрессивность
        QImage res = input;

        // Матрица свертки (упрощенная)
        float kernel[3][3] = {
            { 0, -strength, 0 },
            { -strength, 1.0f + 4.0f * strength, -strength },
            { 0, -strength, 0 }
        };

        for (int y = 1; y < input.height() - 1; ++y) {
            QRgb *outLine = reinterpret_cast<QRgb*>(res.scanLine(y));
            for (int x = 1; x < input.width() - 1; ++x) {
                if (qAlpha(input.pixel(x, y)) == 0) continue;
                float r = 0, g = 0, b = 0, a = qAlpha(input.pixel(x, y));
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        QRgb px = input.pixel(x + kx, y + ky);
                        float w = kernel[ky + 1][kx + 1];
                        r += qRed(px) * w; g += qGreen(px) * w; b += qBlue(px) * w;
                    }
                }
                outLine[x] = qRgba(qBound(0, (int)r, 255), qBound(0, (int)g, 255), qBound(0, (int)b, 255), a);
            }
        }
        return res;
    }
};

// --- Factory Implementation (C++11 compatible) ---
std::unique_ptr<BaseFilter> FilterFactory::createFilter(FilterType type) {
    switch (type) {
    case FilterType::Grayscale: return std::unique_ptr<BaseFilter>(new GrayscaleFilter());
    case FilterType::Invert: return std::unique_ptr<BaseFilter>(new InvertFilter());
    case FilterType::BrightnessContrast: return std::unique_ptr<BaseFilter>(new BrightnessContrastFilter());
    case FilterType::Blur: return std::unique_ptr<BaseFilter>(new BlurFilter());
    case FilterType::Threshold: return std::unique_ptr<BaseFilter>(new ThresholdFilter());
    case FilterType::HSL: return std::unique_ptr<BaseFilter>(new HSLFilter());
    case FilterType::Sharpen: return std::unique_ptr<BaseFilter>(new SharpenFilter());
    default: return std::unique_ptr<BaseFilter>(new NoneFilter());
    }
}

std::vector<FilterParamInfo> FilterFactory::getParamInfo(FilterType type) {
    auto filter = createFilter(type);
    return filter->getParamInfo();
}

FilterState FilterFactory::getDefaultState(FilterType type) {
    FilterState state;
    state.type = type;
    for (const auto& p : getParamInfo(type)) state.params.push_back(p.defaultVal);
    return state;
}