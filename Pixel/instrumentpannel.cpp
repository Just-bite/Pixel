#include "instrumentpannel.h"

InstrumentPannel::InstrumentPannel(QWidget* parent)
    : instrument_pannel_layout(new QVBoxLayout(parent))
{

    instrument_pannel_layout->setContentsMargins(2,2,2,2);
    instrument_pannel_layout->setSpacing(2);
    fillInstumentIcon();
    bttns_instruments.clear();
    createButtonsArray(parent);
    setButtonsIcons();
    addInsruments();

}

const QVBoxLayout& InstrumentPannel::getLayout() const
{
    return *instrument_pannel_layout;
}

void InstrumentPannel::fillInstumentIcon()
{
    instrument_icon[InstumentType::HAND] = {QIcon(":/application_icon/main_icon.jpg"), QString("Hand (H)")};
    instrument_icon[InstumentType::PENCIL] = {QIcon(":/application_icon/main_icon.jpg"), QString("Pencil (P)")};
    instrument_icon[InstumentType::RUBBER] = {QIcon(":/application_icon/rubber.png"), QString("Eraser (E)")};
    instrument_icon[InstumentType::SCISSORS] = {QIcon(":/application_icon/scissors.jpg"), QString("Scissors ()")};
    instrument_icon[InstumentType::FILL] = {QIcon(":/application_icon/main_icon.jpg"), QString("Fill (B)")};
    instrument_icon[InstumentType::POINTER] = {QIcon(":/application_icon/main_icon.jpg"), QString("Pointer (I)")};
    instrument_icon[InstumentType::FIGURE] = {QIcon(":/application_icon/main_icon.jpg"), QString("Figure (F)")};
}

void InstrumentPannel::createButtonsArray(QWidget* parent)
{
    for(size_t i = 0; i < 16; i++)
    {
        QPushButton* button = new QPushButton(QString("%1").arg(i),parent);
        button->setFixedSize(32,32);
        bttns_instruments.push_back(button);
    }
}

void InstrumentPannel::setButtonsIcons()
{
    for(size_t i = 0; i < bttns_instruments.size(); i++)
    {
        InstumentType type = InstumentType::RUBBER;
        bttns_instruments[i]->setIcon(instrument_icon[static_cast<InstumentType>(static_cast<int>(type) + i)].first);
        bttns_instruments[i]->setToolTip(instrument_icon[static_cast<InstumentType>(static_cast<int>(type) + i)].second);
    }
}

void InstrumentPannel::addInsruments()
{
    for(size_t i = 0; i < bttns_instruments.size(); i++)
        instrument_pannel_layout->addWidget(bttns_instruments[i]);
    instrument_pannel_layout->addStretch();
}


