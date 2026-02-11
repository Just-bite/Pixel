#ifndef INSTRUMENTPANNEL_H
#define INSTRUMENTPANNEL_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QString>
#include <unordered_map>

enum class InstumentType
{
    RUBBER,
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
    explicit InstrumentPannel(QWidget* parent = nullptr);
    const QVBoxLayout& getLayout() const;
private:
    void createButtonsArray(QWidget* parent);
    void fillInstumentIcon();
    void setButtonsIcons();
    void addInsruments();

    static constexpr int MAX_INSTRUMENTS_AMOUNT = 32;
    QVBoxLayout* instrument_pannel_layout;

    std::vector<QPushButton*> bttns_instruments;
    std::unordered_map<InstumentType,std::pair<QIcon,QString>> instrument_icon;
};

#endif // INSTRUMENTPANNEL_H
