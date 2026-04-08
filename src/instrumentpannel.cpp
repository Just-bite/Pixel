#include "include\instrumentpannel.h"

#define RESOURCE_PATH_PREFIX ":/application_icon/resources/"

InstrumentPannel::InstrumentPannel(QWidget* parent)
    : QWidget(parent)
    , m_instrument_pannel_layout(new QVBoxLayout(parent))
    , m_button_group(new QButtonGroup(parent))
{
    m_instrument_pannel_layout->setContentsMargins(2,2,2,2);
    m_instrument_pannel_layout->setSpacing(2);

    m_button_group->setExclusive(true);

    fillInstumentIcon();
    m_bttns_instruments.clear();
    createButtonsArray(parent);
    setButtonsIcons();
    addInsruments();

    connect(m_button_group, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [this](int id){
        emit instrumentSelected(static_cast<InstrumentType>(id));
    });

    if (QPushButton* ptrBtn = static_cast<QPushButton*>(m_button_group->button(static_cast<int>(InstrumentType::POINTER)))) {
        ptrBtn->setChecked(true);
    }
}

const QVBoxLayout& InstrumentPannel::getLayout() const { return *m_instrument_pannel_layout; }

void InstrumentPannel::setActiveTool(InstrumentType type) {
    if (QAbstractButton* btn = m_button_group->button(static_cast<int>(type))) {
        btn->setChecked(true);
    }
}

void InstrumentPannel::fillInstumentIcon()
{
    m_instrument_icon[InstrumentType::HAND] = {QIcon(RESOURCE_PATH_PREFIX "hand.svg"), QString("Hand (H)")};
    m_instrument_icon[InstrumentType::PENCIL] = {QIcon(RESOURCE_PATH_PREFIX "pencil.svg"), QString("Pencil (P)")};
    m_instrument_icon[InstrumentType::ERASER] = {QIcon(RESOURCE_PATH_PREFIX "rubber.svg"), QString("Eraser (E)")};
    m_instrument_icon[InstrumentType::SCISSORS] = {QIcon(RESOURCE_PATH_PREFIX "scissors.svg"), QString("Scissors")};
    m_instrument_icon[InstrumentType::FILL] = {QIcon(RESOURCE_PATH_PREFIX "bucket.svg"), QString("Fill (B)")};
    m_instrument_icon[InstrumentType::POINTER] = {QIcon(RESOURCE_PATH_PREFIX "arrow.svg"), QString("Pointer (I)")};
    m_instrument_icon[InstrumentType::TEXT] = {QIcon(RESOURCE_PATH_PREFIX "font.svg"), QString("Text (T)")};
    m_instrument_icon[InstrumentType::FIGURE] = {QIcon(RESOURCE_PATH_PREFIX "figure.svg"), QString("Figure (F)")};
    m_instrument_icon[InstrumentType::PIPETTE] = {QIcon(RESOURCE_PATH_PREFIX "pipette.svg"), QString("Pipette (Alt)")};
}

void InstrumentPannel::createButtonsArray(QWidget* parent)
{
    for(size_t i = 0; i < MAX_INSTRUMENTS_AMOUNT; i++)
    {
        QPushButton* button = new QPushButton(parent);
        button->setFixedSize(INSTURMENT_BTN_SIZE, INSTURMENT_BTN_SIZE);
        button->setCheckable(true);

        m_bttns_instruments.push_back(button);
        m_button_group->addButton(button, i);
    }
}

void InstrumentPannel::setButtonsIcons()
{
    for(size_t i = 0; i < m_bttns_instruments.size(); i++)
    {
        InstrumentType type = static_cast<InstrumentType>(i);

        if (type == InstrumentType::SCISSORS) {
            m_bttns_instruments[i]->hide();
            continue;
        }

        if (m_instrument_icon.find(type) != m_instrument_icon.end()) {
            m_bttns_instruments[i]->setIcon(m_instrument_icon[type].first);
            m_bttns_instruments[i]->setToolTip(m_instrument_icon[type].second);

            if (type != InstrumentType::POINTER && type != InstrumentType::HAND &&
                type != InstrumentType::FIGURE && type != InstrumentType::TEXT &&
                type != InstrumentType::PENCIL && type != InstrumentType::ERASER &&
                type != InstrumentType::FILL && type != InstrumentType::PIPETTE) {
                m_bttns_instruments[i]->setEnabled(false);
                m_bttns_instruments[i]->setToolTip(m_instrument_icon[type].second + " (Not implemented)");
            }
        } else {
            m_bttns_instruments[i]->hide();
        }
    }
}

void InstrumentPannel::addInsruments()
{
    for(size_t i = 0; i < m_bttns_instruments.size(); i++)
        m_instrument_pannel_layout->addWidget(m_bttns_instruments[i]);
    m_instrument_pannel_layout->addStretch();
}