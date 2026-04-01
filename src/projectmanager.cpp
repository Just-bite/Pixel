#include "include\projectmanager.h"
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QImage>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QBuffer>
#include <QByteArray>

ProjectManager::ProjectManager(QWidget* parent) : QWidget(parent), m_selected_project(nullptr) {}

void ProjectManager::createProject() {
    Project* project = new Project();
    m_projects.push_back(project);
    if (m_projects.size() == 1) m_selected_project = project;
    m_current_file_path = "";
}

Canvas* ProjectManager::GetCurrentCanvas() {
    if (!m_selected_project) return nullptr;
    return m_selected_project->GetCanvas();
}

bool ProjectManager::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Pixel Project", "", "Pixel Files (*.pxl)");
    if (fileName.isEmpty()) return false;
    loadFromJson(fileName);
    return true;
}

bool ProjectManager::saveFile() {
    if (m_current_file_path.isEmpty()) return saveAsFile();
    saveToJson(m_current_file_path);
    return true;
}

bool ProjectManager::saveAsFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Pixel Project As", "", "Pixel Files (*.pxl)");
    if (fileName.isEmpty()) return false;
    if (!fileName.endsWith(".pxl")) fileName += ".pxl";
    saveToJson(fileName);
    return true;
}

bool ProjectManager::exportPng() {
    Canvas* canvas = GetCurrentCanvas();
    if (!canvas || !canvas->getScene()) return false;

    QString fileName = QFileDialog::getSaveFileName(this, "Export to PNG", "", "Images (*.png)");
    if (fileName.isEmpty()) return false;
    if (!fileName.endsWith(".png")) fileName += ".png";

    QImage image(canvas->getSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    canvas->getScene()->render(&painter, QRectF(), QRectF(0, 0, canvas->getSize().width(), canvas->getSize().height()));

    return image.save(fileName);
}

void ProjectManager::saveToJson(const QString& path) {
    Canvas* canvas = GetCurrentCanvas();
    if (!canvas) return;

    QJsonObject root;
    root["version"] = 1;

    QJsonArray layersArr;
    for (Layer* l : canvas->getLayers()) {
        QJsonObject lObj;
        lObj["name"] = l->getName();
        lObj["visible"] = l->isVisible();
        lObj["locked"] = l->isLocked();
        lObj["isFilter"] = l->isFilter();

        if (l->isFilter()) {
            FilterLayer* fl = static_cast<FilterLayer*>(l);
            FilterState s = fl->getFilterState();
            lObj["f_type"] = static_cast<int>(s.type);
            lObj["f_p1"] = s.param1;
            lObj["f_p2"] = s.param2;
        }

        QJsonArray objsArr;
        for (Object* o : l->getObjects()) {
            QJsonObject sObj;
            if (Figure* f = dynamic_cast<Figure*>(o)) {
                sObj["class"] = "figure";
                FigureState s = f->getState();
                sObj["type"] = static_cast<int>(s.type);
                sObj["px"] = s.pos.x(); sObj["py"] = s.pos.y();
                sObj["x"] = s.rect.x(); sObj["y"] = s.rect.y();
                sObj["w"] = s.rect.width(); sObj["h"] = s.rect.height();
                sObj["rot"] = s.rot; sObj["thick"] = s.thickness;
                sObj["fill"] = s.fill.name(QColor::HexArgb);
                sObj["stroke"] = s.stroke.name(QColor::HexArgb);
            } else if (TextObject* t = dynamic_cast<TextObject*>(o)) {
                sObj["class"] = "text";
                TextState s = t->getState();
                sObj["text"] = s.text;
                sObj["font"] = s.font.toString();
                sObj["color"] = s.color.name(QColor::HexArgb);
                sObj["px"] = s.pos.x(); sObj["py"] = s.pos.y();
                sObj["x"] = s.rect.x(); sObj["y"] = s.rect.y();
                sObj["w"] = s.rect.width(); sObj["h"] = s.rect.height();
                sObj["rot"] = s.rot;
            } else if (ImageObject* iObj = dynamic_cast<ImageObject*>(o)) {
                sObj["class"] = "image";
                ImageState s = iObj->getState();
                sObj["px"] = s.pos.x(); sObj["py"] = s.pos.y();
                sObj["x"] = s.rect.x(); sObj["y"] = s.rect.y();
                sObj["w"] = s.rect.width(); sObj["h"] = s.rect.height();
                sObj["rot"] = s.rot;
                if (!s.image.isNull()) {
                    QByteArray byteArray; QBuffer buffer(&byteArray);
                    buffer.open(QIODevice::WriteOnly); s.image.save(&buffer, "PNG");
                    sObj["image"] = QString::fromLatin1(byteArray.toBase64());
                }
            }
            objsArr.append(sObj);
        }
        lObj["objects"] = objsArr;
        layersArr.append(lObj);
    }
    root["layers"] = layersArr;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        m_current_file_path = path;
    }
}

void ProjectManager::loadFromJson(const QString& path) {
    Canvas* canvas = GetCurrentCanvas();
    if (!canvas) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return;

    emit projectAboutToClose();
    canvas->clearCanvas();

    QJsonArray layersArr = doc.object()["layers"].toArray();
    for (int i = 0; i < layersArr.size(); ++i) {
        QJsonObject lObj = layersArr[i].toObject();
        bool isFilter = lObj["isFilter"].toBool(false);
        Layer* l = nullptr;

        if (isFilter) {
            FilterLayer* fl = new FilterLayer(lObj["name"].toString());
            FilterState fs;
            fs.type = static_cast<FilterType>(lObj["f_type"].toInt(0));
            fs.param1 = lObj["f_p1"].toDouble(0);
            fs.param2 = lObj["f_p2"].toDouble(0);
            fl->setFilterState(fs);
            l = fl;
        } else {
            l = new Layer(lObj["name"].toString());
        }

        l->setVisible(lObj["visible"].toBool(true));
        canvas->addLayer(l);

        QJsonArray objsArr = lObj["objects"].toArray();
        for (int j = 0; j < objsArr.size(); ++j) {
            QJsonObject sObj = objsArr[j].toObject();
            QString cls = sObj["class"].toString("figure");

            if (cls == "figure") {
                Figure* f = new Figure();
                FigureState s;
                s.type = static_cast<FigureType>(sObj["type"].toInt());
                s.pos = QPointF(sObj["px"].toDouble(), sObj["py"].toDouble());
                s.rect = QRectF(sObj["x"].toDouble(), sObj["y"].toDouble(), sObj["w"].toDouble(), sObj["h"].toDouble());
                s.rot = sObj["rot"].toDouble();
                s.thickness = sObj["thick"].toDouble();
                s.fill = QColor(sObj["fill"].toString());
                s.stroke = QColor(sObj["stroke"].toString());
                f->setState(s);
                l->addObject(f);
            } else if (cls == "text") {
                TextObject* t = new TextObject();
                TextState s;
                s.text = sObj["text"].toString();
                s.font.fromString(sObj["font"].toString());
                s.color = QColor(sObj["color"].toString());
                s.pos = QPointF(sObj["px"].toDouble(), sObj["py"].toDouble());
                s.rect = QRectF(sObj["x"].toDouble(), sObj["y"].toDouble(), sObj["w"].toDouble(), sObj["h"].toDouble());
                s.rot = sObj["rot"].toDouble();
                t->setState(s);
                l->addObject(t);
            } else if (cls == "image") {
                ImageObject* imgObj = new ImageObject();
                ImageState s;
                s.pos = QPointF(sObj["px"].toDouble(), sObj["py"].toDouble());
                s.rect = QRectF(sObj["x"].toDouble(), sObj["y"].toDouble(), sObj["w"].toDouble(), sObj["h"].toDouble());
                s.rot = sObj["rot"].toDouble();
                if (sObj.contains("image")) {
                    QByteArray byteArray = QByteArray::fromBase64(sObj["image"].toString().toLatin1());
                    s.image.loadFromData(byteArray, "PNG");
                }
                imgObj->setState(s);
                l->addObject(imgObj);
            }
        }
        l->setLocked(lObj["locked"].toBool(false));
    }

    m_current_file_path = path;
    canvas->renderCanvas();
    canvas->updateFilters();
    emit projectLoaded();
    emit layersUpdated();
}

bool ProjectManager::createFile() {
    QDialog dlg(this);
    dlg.setWindowTitle("New Project");

    QFormLayout* form = new QFormLayout(&dlg);
    QSpinBox* wBox = new QSpinBox(&dlg); wBox->setRange(10, 10000); wBox->setValue(800);
    QSpinBox* hBox = new QSpinBox(&dlg); hBox->setRange(10, 10000); hBox->setValue(600);

    form->addRow("Width:", wBox);
    form->addRow("Height:", hBox);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        Canvas* canvas = GetCurrentCanvas();
        if (canvas) {
            emit projectAboutToClose();
            canvas->clearCanvas();
            canvas->setSize(wBox->value(), hBox->value());
            canvas->newLayer();
            m_current_file_path = "";
            emit projectLoaded();
            emit layersUpdated();
        }
        return true;
    }
    return false;
}