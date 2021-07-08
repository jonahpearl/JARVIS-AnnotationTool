/*****************************************************************
 * File:			newdatasetwindow.cpp
 * Created: 	07. July 2021
 * Author:		Timo Hüser
 * Contact: 	timo.hueser@gmail.com
 *****************************************************************/

#include "newdatasetwindow.hpp"

#include <QGridLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QDirIterator>


NewDatasetWindow::NewDatasetWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
	this->resize(800,800);
	this->setMinimumSize(600,600);
	settings = new QSettings();
	setWindowTitle("New Dataset");
	QGridLayout *layout = new QGridLayout(this);

	m_datasetConfig = new DatasetConfig;
	datasetCreator = new DatasetCreator(m_datasetConfig);

	loadPresetsWindow = new PresetsWindow(&presets, "load", "New Dataset Window/");
	savePresetsWindow = new PresetsWindow(&presets, "save", "New Dataset Window/");
	connect(loadPresetsWindow, SIGNAL(loadPreset(QString)), this, SLOT(loadPresetSlot(QString)));
	connect(savePresetsWindow, SIGNAL(savePreset(QString)), this, SLOT(savePresetSlot(QString)));

	QGroupBox *configBox = new QGroupBox("Configuration");
	configBox->setMinimumSize(0,180);
	QGridLayout *configlayout = new QGridLayout(configBox);
	LabelWithToolTip *datasetNameLabel = new LabelWithToolTip("New Dataset Name", "");
	datasetNameEdit = new QLineEdit(m_datasetConfig->datasetName, configBox);
	connect (datasetNameEdit, &QLineEdit::textEdited, this, &NewDatasetWindow::datasetNameChangedSlot);
	LabelWithToolTip *datatypeLabel = new LabelWithToolTip("Datatype to Load", "kdsff");
	videosButton = new QRadioButton("Videos", configBox);
	videosButton->setChecked(m_datasetConfig->dataType == "Videos");
	connect(videosButton, &QRadioButton::toggled, this, &NewDatasetWindow::dataTypeChangedSlot);
	imagesButton = new QRadioButton("Images", configBox);
	imagesButton->setChecked(m_datasetConfig->dataType == "Images");
	LabelWithToolTip *numCamerasLabel = new LabelWithToolTip("Number of Cameras", "kdsff");
	numCamerasBox = new QSpinBox(configBox);
	numCamerasBox->setMinimum(0);
	numCamerasBox->setValue(m_datasetConfig->numCameras);
	connect(numCamerasBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NewDatasetWindow::numCamerasChangedSlot);
	LabelWithToolTip *frameSetsRecordingLabel = new LabelWithToolTip("Frames per Recording", "kdsff");
	frameSetsRecordingBox = new QSpinBox(configBox);
	frameSetsRecordingBox->setMinimum(0);
	frameSetsRecordingBox->setMaximum(99999);
	frameSetsRecordingBox->setValue(m_datasetConfig->frameSetsRecording);
	connect(frameSetsRecordingBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NewDatasetWindow::frameSetsRecordingChandedSlot);
	LabelWithToolTip *samplingMethodLabel = new LabelWithToolTip("Sampling Method", "kdsff");
	samplingMethodCombo = new QComboBox(configBox);
	samplingMethodCombo->addItem("uniform");
	samplingMethodCombo->addItem("kmeans");
	samplingMethodCombo->setCurrentText(m_datasetConfig->samplingMethod);
	connect(samplingMethodCombo, &QComboBox::currentTextChanged, this, &NewDatasetWindow::samplingMethodChangedSlot);

	QGroupBox *recordingsBox = new QGroupBox("Recordings");
	QGridLayout *recordingslayout = new QGridLayout(recordingsBox);
	recordingsItemList = new ConfigurableItemList("Recordings", m_datasetConfig, true);
	recordingslayout->addWidget(recordingsItemList,0,0);
	recordingslayout->setMargin(0);

	QGroupBox *entitiesBox = new QGroupBox("Entities");
	QGridLayout *entitieslayout = new QGridLayout(entitiesBox);
	entitiesItemList = new ConfigurableItemList("Entities", m_datasetConfig);
	entitieslayout->addWidget(entitiesItemList,0,0);
	entitieslayout->setMargin(0);

	QGroupBox *keypointsBox = new QGroupBox("Keypoints");
	QGridLayout *keypointslayout = new QGridLayout(keypointsBox);
	keypointsItemList = new ConfigurableItemList("Keypoints", m_datasetConfig);
	keypointslayout->addWidget(keypointsItemList,0,0);
	keypointslayout->setMargin(0);

	QWidget *buttonBarWidget = new QWidget(this);
	buttonBarWidget->setMaximumSize(100000,50);
	QGridLayout *buttonbarlayout = new QGridLayout(buttonBarWidget);
	saveButton = new QPushButton("Save");
	saveButton->setIcon(QIcon::fromTheme("upload"));
	saveButton->setMinimumSize(40,40);
	connect(saveButton, &QPushButton::clicked, this, &NewDatasetWindow::savePresetsClickedSlot);
	loadButton = new QPushButton("Load");
	loadButton->setIcon(QIcon::fromTheme("download"));
	loadButton->setMinimumSize(40,40);
	connect(loadButton, &QPushButton::clicked, this, &NewDatasetWindow::loadPresetsClickedSlot);
	QWidget *middleSpacer = new QWidget();
	middleSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	createButton = new QPushButton("Create");
	createButton->setMinimumSize(40,40);
	connect(createButton, &QPushButton::clicked, this, &NewDatasetWindow::createDatasetClickedSlot);
	buttonbarlayout->addWidget(saveButton, 0,0);
	buttonbarlayout->addWidget(loadButton,0,1);
	buttonbarlayout->addWidget(middleSpacer,0,2);
	buttonbarlayout->addWidget(createButton,0,3);

	configlayout->addWidget(datasetNameLabel,0,0);
	configlayout->addWidget(datasetNameEdit,0,1,1,2);
	configlayout->addWidget(datatypeLabel,1,0);
	configlayout->addWidget(videosButton,1,1);
	configlayout->addWidget(imagesButton,1,2);
	configlayout->addWidget(numCamerasLabel,2,0);
	configlayout->addWidget(numCamerasBox,2,1,1,2);
	configlayout->addWidget(frameSetsRecordingLabel,3,0);
	configlayout->addWidget(frameSetsRecordingBox,3,1,1,2);
	configlayout->addWidget(samplingMethodLabel,4,0);
	configlayout->addWidget(samplingMethodCombo,4,1,1,2);

	layout->addWidget(configBox,0,0,1,2);
	layout->addWidget(recordingsBox,1,0,2,1);
	layout->addWidget(entitiesBox,1,1);
	layout->addWidget(keypointsBox,2,1);
	layout->addWidget(buttonBarWidget,3,0,1,2);

	connect(this, &NewDatasetWindow::createDataset, datasetCreator, &DatasetCreator::createDatasetSlot);
}

void NewDatasetWindow::datasetNameChangedSlot(const QString &name) {
	m_datasetConfig->datasetName = name;
}

void NewDatasetWindow::dataTypeChangedSlot() {
	if (videosButton->isChecked()) {
		m_datasetConfig->dataType = "Videos";
	}
	else {
		m_datasetConfig->dataType = "Images";
	}
}

void NewDatasetWindow::numCamerasChangedSlot(int num) {
	m_datasetConfig->numCameras = num;
}

void NewDatasetWindow::frameSetsRecordingChandedSlot(int num) {
	m_datasetConfig->frameSetsRecording = num;
}

void NewDatasetWindow::samplingMethodChangedSlot(const QString &method) {
	m_datasetConfig->samplingMethod = method;
}

void NewDatasetWindow::createDatasetClickedSlot() {
	emit createDataset(recordingsItemList->getItems(), entitiesItemList->getItems(), keypointsItemList->getItems());
}

void NewDatasetWindow::savePresetsClickedSlot() {
	savePresetsWindow->updateListSlot();
	savePresetsWindow->show();
}


void NewDatasetWindow::loadPresetsClickedSlot() {
	loadPresetsWindow->updateListSlot();
	loadPresetsWindow->show();
}


void NewDatasetWindow::savePresetSlot(const QString& preset) {
	settings->beginGroup(preset);
	settings->beginGroup("entitiesItemList");
	QList<QString> entItemsList;
	for (const auto& item : entitiesItemList->itemSelectorList->findItems("",Qt::MatchContains)) {
		entItemsList.append(item->text());
	}
	settings->setValue("itemsList", QVariant::fromValue(entItemsList));
	settings->endGroup();
	settings->beginGroup("keypointItemList");
	QList<QString> keyItemsList;
	for (const auto& item : keypointsItemList->itemSelectorList->findItems("",Qt::MatchContains)) {
		keyItemsList.append(item->text());
	}
	settings->setValue("itemsList", QVariant::fromValue(keyItemsList));
	settings->endGroup();
	settings->endGroup();
}


void NewDatasetWindow::loadPresetSlot(const QString& preset) {
	entitiesItemList->itemSelectorList->clear();
	keypointsItemList->itemSelectorList->clear();
	settings->beginGroup(preset);
	settings->beginGroup("entitiesItemList");
	QList<QString>entIitemsList = settings->value("itemsList").value<QList<QString>>();
	for (const auto& item : entIitemsList) {
		entitiesItemList->itemSelectorList->addItem(item);
	}
	settings->endGroup();
	settings->beginGroup("keypointItemList");
	QList<QString> keyItemsList = settings->value("itemsList").value<QList<QString>>();
	for (const auto& item : keyItemsList) {
		keypointsItemList->itemSelectorList->addItem(item);
	}
	settings->endGroup();
	settings->endGroup();
}

LabelWithToolTip::LabelWithToolTip(QString name, QString toolTip, QWidget *parent) : QWidget(parent) {
	QGridLayout *layout = new QGridLayout(this);
	layout->setMargin(0);
	QLabel *label = new QLabel(name);
	if (toolTip != "") {
		QLabel *helpIcon = new QLabel(this);
		helpIcon->setPixmap((QIcon::fromTheme("info2")).pixmap(15,15));
		helpIcon->setToolTip(toolTip);
		helpIcon->setMaximumSize(10,40);
		layout->addWidget(helpIcon,0,1, Qt::AlignLeft);
	}
	layout->addWidget(label,0,0);
}
