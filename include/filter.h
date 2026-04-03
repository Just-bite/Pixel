#ifndef FILTER_H
#define FILTER_H

#include <QString>
#include <QImage>
#include <vector>
#include <memory>
#include <cmath>

    enum class FilterType { None, Grayscale, Invert, BrightnessContrast, Blur, Threshold, Sharpen, HSL };

struct FilterParamInfo {
    QString name;
    float minVal;
    float maxVal;
    float defaultVal;
};

struct FilterState {
    FilterType type = FilterType::None;
    std::vector<float> params;

    bool operator==(const FilterState& o) const {
        if (type != o.type || params.size() != o.params.size()) return false;
        for (size_t i = 0; i < params.size(); ++i) {
            if (std::abs(params[i] - o.params[i]) > 0.01f) return false;
        }
        return true;
    }
    bool operator!=(const FilterState& o) const { return !(*this == o); }
};

class BaseFilter {
public:
    virtual ~BaseFilter() = default;
    virtual FilterType getType() const = 0;
    virtual std::vector<FilterParamInfo> getParamInfo() const = 0;
    virtual QImage apply(const QImage& input, const std::vector<float>& params) = 0;
};

class FilterFactory {
public:
    static std::unique_ptr<BaseFilter> createFilter(FilterType type);
    static std::vector<FilterParamInfo> getParamInfo(FilterType type);
    static FilterState getDefaultState(FilterType type);
};

#endif // FILTER_H