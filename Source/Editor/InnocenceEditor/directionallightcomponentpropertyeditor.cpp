#include "directionallightcomponentpropertyeditor.h"

#include "../../Engine/ModuleManager/IModuleManager.h"

INNO_ENGINE_API extern IModuleManager* g_pModuleManager;

DirectionalLightComponentPropertyEditor::DirectionalLightComponentPropertyEditor()
{
}

void DirectionalLightComponentPropertyEditor::initialize()
{
	m_gridLayout = new QGridLayout();
	m_gridLayout->setMargin(4);

	m_validator = new QDoubleValidator(-2147483647, 2147483647, 4);
	m_validator->setProperty("notation", QDoubleValidator::StandardNotation);

	m_title = new QLabel("DirectionalLightComponent");
	m_title->setStyleSheet(
		"background-repeat: no-repeat;"
		"background-position: left;"
		"padding-left: 20px;"
	);

	m_line = new QWidget();
	m_line->setFixedHeight(1);
	m_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_line->setStyleSheet(QString("background-color: #585858;"));

	int row = 0;
	m_gridLayout->addWidget(m_title, row, 0, 1, 7);
	row++;

	m_gridLayout->addWidget(m_line, row, 0, 1, 7);

	m_gridLayout->setHorizontalSpacing(m_horizontalSpacing);
	m_gridLayout->setVerticalSpacing(m_verticalSpacing);

	this->setLayout(m_gridLayout);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->hide();
}

void DirectionalLightComponentPropertyEditor::edit(void *component)
{
	m_component = reinterpret_cast<DirectionalLightComponent*>(component);

	this->show();
}

void DirectionalLightComponentPropertyEditor::remove()
{
	m_component = nullptr;
	this->hide();
}