#include "instrumentpannel.h"

InstrumentPannel::InstrumentPannel(QWidget* parent/*, ProjectManager* manager*/)
    : m_instrument_pannel_layout(new QVBoxLayout(parent))
    //, m_project_manager(manager)
{
    m_instrument_pannel_layout->setContentsMargins(2,2,2,2);
    m_instrument_pannel_layout->setSpacing(2);
    fillInstumentIcon();
    m_bttns_instruments.clear();
    createButtonsArray(parent);
    setButtonsIcons();
    addInsruments();
}

const QVBoxLayout& InstrumentPannel::getLayout() const
{
    return *m_instrument_pannel_layout;
}

void InstrumentPannel::fillInstumentIcon()
{
    m_instrument_icon[InstrumentType::HAND] = {QIcon(":/application_icon/hand.svg"), QString("Hand (H)")};
    m_instrument_icon[InstrumentType::PENCIL] = {QIcon(":/application_icon/pencil.svg"), QString("Pencil (P)")};
    m_instrument_icon[InstrumentType::ERASER] = {QIcon(":/application_icon/rubber.png"), QString("Eraser (E)")};
    m_instrument_icon[InstrumentType::SCISSORS] = {QIcon(":/application_icon/scissors.svg"), QString("Scissors ()")};
    m_instrument_icon[InstrumentType::FILL] = {QIcon(":/application_icon/bucket.svg"), QString("Fill (B)")};
    m_instrument_icon[InstrumentType::POINTER] = {QIcon(":/application_icon/arrow.svg"), QString("Pointer (I)")};
    m_instrument_icon[InstrumentType::FIGURE] = {QIcon(":/application_icon/figure.svg"), QString("Figure (F)")};
}

void InstrumentPannel::createButtonsArray(QWidget* parent)
{
    for(size_t i = 0; i < MAX_INSTRUMENTS_AMOUNT; i++)
    {
        QPushButton* button = new QPushButton(/*QString("%1").arg(i),*/parent);
        button->setFixedSize(INSTURMENT_BTN_SIZE, INSTURMENT_BTN_SIZE);
        //connect(button,&QPushButton::clicked,mana
        m_bttns_instruments.push_back(button);
    }
}

void InstrumentPannel::setButtonsIcons()
{
    for(size_t i = 0; i < m_bttns_instruments.size(); i++)
    {
        InstrumentType type = InstrumentType::ERASER;
        m_bttns_instruments[i]->setIcon(m_instrument_icon[static_cast<InstrumentType>(static_cast<int>(type) + i)].first);
        m_bttns_instruments[i]->setToolTip(m_instrument_icon[static_cast<InstrumentType>(static_cast<int>(type) + i)].second);
    }
}

void InstrumentPannel::addInsruments()
{
    for(size_t i = 0; i < m_bttns_instruments.size(); i++)
        m_instrument_pannel_layout->addWidget(m_bttns_instruments[i]);
    m_instrument_pannel_layout->addStretch();
}


