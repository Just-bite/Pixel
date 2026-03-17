#include "instrumentpannel.h"

InstrumentPannel::InstrumentPannel(QWidget* parent)
    : QWidget(parent)
    // ВОЗВРАЩАЕМ PARENT: строим интерфейс на виджете, который передал MainWindow
    , m_instrument_pannel_layout(new QVBoxLayout(parent))
    , m_button_group(new QButtonGroup(parent))
{
    m_instrument_pannel_layout->setContentsMargins(2,2,2,2);
    m_instrument_pannel_layout->setSpacing(2);

    // Включаем эксклюзивный режим (только 1 кнопка может быть нажата)
    m_button_group->setExclusive(true);

    fillInstumentIcon();
    m_bttns_instruments.clear();
    createButtonsArray(parent); // Передаем parent внутрь
    setButtonsIcons();
    addInsruments();

    // Связываем клик по кнопке с отправкой сигнала
    connect(m_button_group, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [this](int id){
        emit instrumentSelected(static_cast<InstrumentType>(id));
    });

    // Включаем Pointer по умолчанию
    if (QPushButton* ptrBtn = static_cast<QPushButton*>(m_button_group->button(static_cast<int>(InstrumentType::POINTER)))) {
        ptrBtn->setChecked(true);
    }
}

const QVBoxLayout& InstrumentPannel::getLayout() const { return *m_instrument_pannel_layout; }

void InstrumentPannel::fillInstumentIcon()
{
    m_instrument_icon[InstrumentType::HAND] = {QIcon(":/application_icon/hand.svg"), QString("Hand (H)")};
    m_instrument_icon[InstrumentType::PENCIL] = {QIcon(":/application_icon/pencil.svg"), QString("Pencil (P)")};
    m_instrument_icon[InstrumentType::ERASER] = {QIcon(":/application_icon/rubber.png"), QString("Eraser (E)")};
    m_instrument_icon[InstrumentType::SCISSORS] = {QIcon(":/application_icon/scissors.svg"), QString("Scissors")};
    m_instrument_icon[InstrumentType::FILL] = {QIcon(":/application_icon/bucket.svg"), QString("Fill (B)")};
    m_instrument_icon[InstrumentType::POINTER] = {QIcon(":/application_icon/arrow.svg"), QString("Pointer (I)")};
    m_instrument_icon[InstrumentType::FIGURE] = {QIcon(":/application_icon/figure.svg"), QString("Figure (F)")};
}

void InstrumentPannel::createButtonsArray(QWidget* parent)
{
    for(size_t i = 0; i < MAX_INSTRUMENTS_AMOUNT; i++)
    {
        // Кнопки тоже должны лежать на parent
        QPushButton* button = new QPushButton(parent);
        button->setFixedSize(INSTURMENT_BTN_SIZE, INSTURMENT_BTN_SIZE);
        button->setCheckable(true); // Кнопку можно "зажать"

        m_bttns_instruments.push_back(button);
        m_button_group->addButton(button, i); // ID кнопки = номер в Enum
    }
}

void InstrumentPannel::setButtonsIcons()
{
    for(size_t i = 0; i < m_bttns_instruments.size(); i++)
    {
        InstrumentType type = static_cast<InstrumentType>(i);

        if (m_instrument_icon.find(type) != m_instrument_icon.end()) {
            m_bttns_instruments[i]->setIcon(m_instrument_icon[type].first);
            m_bttns_instruments[i]->setToolTip(m_instrument_icon[type].second);

            // Если это не Указатель, Рука или Фигура - отключаем кнопку
            if (type != InstrumentType::POINTER && type != InstrumentType::HAND && type != InstrumentType::FIGURE) {
                m_bttns_instruments[i]->setEnabled(false);
                m_bttns_instruments[i]->setToolTip(m_instrument_icon[type].second + " (Not implemented)");
            }
        } else {
            // Если иконки нет - прячем кнопку вообще
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