#ifndef INSTRUMENTPANNEL_H
#define INSTRUMENTPANNEL_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QString>
#include <unordered_map>

#include "contextpannel.h"
//#include "projectmanager.h"

enum class InstrumentType
{
    ERASER,
    PENCIL,
    POINTER,
    SCISSORS,
    FILL,
    HAND,
    FIGURE,
};


class InstrumentPannel : public QWidget
{
    Q_OBJECT
public:
    explicit InstrumentPannel(QWidget* parent = nullptr/*, ProjectManager* manager = nullptr*/);
    const QVBoxLayout& getLayout() const;
private:
    void createButtonsArray(QWidget* parent);
    void fillInstumentIcon();
    void setButtonsIcons();
    void addInsruments();

    static constexpr int MAX_INSTRUMENTS_AMOUNT = 16;
    static constexpr int INSTURMENT_BTN_SIZE = 32;
    QVBoxLayout* m_instrument_pannel_layout;
    //ProjectManager* m_project_manager;
    std::vector<QPushButton*> m_bttns_instruments;
    std::unordered_map<InstrumentType,std::pair<QIcon,QString>> m_instrument_icon;
};

#endif // INSTRUMENTPANNEL_H
