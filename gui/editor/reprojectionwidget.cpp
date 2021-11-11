/*****************************************************************
	* File:			  reprojectionwidget.cpp
	* Created: 	  03. December 2020
	* Author:		  Timo Hueser
	* Contact: 	  timo.hueser@gmail.com
	* Copyright:  2021 Timo Hueser
	* License:    GPL v3.0
	*****************************************************************/

#include "reprojectionwidget.hpp"

#include <QFileDialog>


ReprojectionWidget::ReprojectionWidget(QWidget *parent) : QWidget(parent) {
	settings = new QSettings();
	m_colorMap = new ColorMap(ColorMap::Jet);
	QGridLayout *reprojectionlayout = new QGridLayout(this);

	QLabel *reprojectionLabel = new QLabel("Reprojection Tool");
	reprojectionLabel->setFont(QFont("Sans Serif", 12, QFont::Bold));
	toggleSwitch = new Switch(this);
	toggleSwitch->setMinimumSize(55,25);
	toggleSwitch->setMaximumSize(55,25);
	toggleSwitch->setEnabled(false);
	connect(toggleSwitch, &Switch::toggled, this, &ReprojectionWidget::switchToggledSlot);

	modeLabel = new QLabel("Mode");
	modeLabel->hide();
	modeCombo = new QComboBox(this);
	modeCombo->addItem("Reprojection");
	modeCombo->addItem("Bone Length");
	connect(modeCombo, &QComboBox::currentTextChanged, this, &ReprojectionWidget::modeComboChangedSlot);
	modeCombo->hide();

	stackedWidget = new QStackedWidget(this);

	QLabel *loadDatasetFirstLabel = new QLabel("Load Datset First...");
	loadDatasetFirstLabel->setFont(QFont("Sans Serif", 14, QFont::Bold));
	loadDatasetFirstLabel->setAlignment(Qt::AlignCenter);
	stackedWidget->addWidget(loadDatasetFirstLabel);

	calibrationSetup = new QWidget(stackedWidget);
	QGridLayout *calibrationsetuplayout = new QGridLayout(calibrationSetup);

	intrinsicsBox = new QGroupBox("Intrinsic Camera Parameters");
	QGridLayout *intrinsicslayout = new QGridLayout(intrinsicsBox);
	QLabel *intrinsicsPathLabel = new QLabel("Intrinsics Path:",intrinsicsBox);
	intrinsicsPathEdit = new QLineEdit(intrinsicsBox);
	intrinsicsPathButton = new QPushButton();
	intrinsicsPathButton->setMinimumSize(30,30);
	intrinsicsPathButton->setIcon(QIcon::fromTheme("folder"));
	connect(intrinsicsPathButton, &QPushButton::clicked, this, &ReprojectionWidget::intrinsicsPathClickedSlot);
	intrinsicslayout->addWidget(intrinsicsPathLabel,0,0,1,2);
	intrinsicslayout->addWidget(intrinsicsPathEdit,1,0);
	intrinsicslayout->addWidget(intrinsicsPathButton,1,2);

	QWidget *spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	extrinsicsBox = new QGroupBox("Extrinsic Camera Parameters");
	QGridLayout *extrinsicslayout = new QGridLayout(extrinsicsBox);
	QWidget *primaryWidget = new QWidget(this);
	QGridLayout *primarylayout = new QGridLayout(primaryWidget);
	QLabel *primaryLabel = new QLabel("Primary");
	primarylayout->setMargin(0);
	primaryCombo = new QComboBox(extrinsicsBox);
	primarylayout->addWidget(primaryLabel,0,0);
	primarylayout->addWidget(primaryCombo,0,1);
	QLabel *extrinsicsPathLabel = new QLabel("Extrinsics Path:" ,extrinsicsBox);
	extrinsicsPathEdit = new QLineEdit(extrinsicsBox);
	extrinsicsPathButton = new QPushButton();
	extrinsicsPathButton->setMinimumSize(30,30);
	extrinsicsPathButton->setIcon(QIcon::fromTheme("folder"));
	connect(extrinsicsPathButton, &QPushButton::clicked, this, &ReprojectionWidget::extrinsicsPathClickedSlot);
	extrinsicslayout->addWidget(primaryWidget,0,0,1,2);
	extrinsicslayout->addWidget(extrinsicsPathLabel,1,0,1,2);
	extrinsicslayout->addWidget(extrinsicsPathEdit,2,0);
	extrinsicslayout->addWidget(extrinsicsPathButton,2,1);
	initReprojectionButton = new QPushButton("Initialise Reprojection Tool");
	connect(initReprojectionButton, &QPushButton::clicked, this, &ReprojectionWidget::initReprojectionClickedSlot);

	calibrationsetuplayout->addWidget(intrinsicsBox,0,0);
	calibrationsetuplayout->addWidget(extrinsicsBox,1,0);
	calibrationsetuplayout->addWidget(initReprojectionButton,2,0,Qt::AlignRight);
	calibrationsetuplayout->addWidget(spacer,3,0);

	reprojectionChartWidget = new ReprojectionChartWidget(this);
	boneLengthChartWidget = new BoneLengthChartWidget(this);

	stackedWidget->addWidget(loadDatasetFirstLabel);
	stackedWidget->addWidget(calibrationSetup);
	stackedWidget->addWidget(reprojectionChartWidget);
	stackedWidget->addWidget(boneLengthChartWidget);

	reprojectionlayout->addWidget(reprojectionLabel,0,0);
	reprojectionlayout->addWidget(toggleSwitch,0,1, Qt::AlignRight);
	reprojectionlayout->addWidget(modeLabel,1,0);
	reprojectionlayout->addWidget(modeCombo,1,1);
	reprojectionlayout->addWidget(stackedWidget,2,0,1,2);

	//--- SIGNAL-SLOT Connections ---//
	//-> Incoming Signals

	//<- Outgoing Signals
	connect(this, &ReprojectionWidget::datasetLoaded, reprojectionChartWidget, &ReprojectionChartWidget::datasetLoadedSlot);
	connect(this, &ReprojectionWidget::datasetLoaded, boneLengthChartWidget, &BoneLengthChartWidget::datasetLoadedSlot);
	connect(this, &ReprojectionWidget::errorThresholdChanged, reprojectionChartWidget, &ReprojectionChartWidget::errorThresholdChanged);
	connect(this, &ReprojectionWidget::boneLengthErrorThresholdChanged, boneLengthChartWidget, &BoneLengthChartWidget::boneLengthErrorThresholdChanged);
	//<-> Relayed Signals
}


void ReprojectionWidget::datasetLoadedSlot() {
	primaryCombo->clear();
	m_numCameras = Dataset::dataset->numCameras();
	m_entitiesList = Dataset::dataset->entitiesList();
	m_bodypartsList = Dataset::dataset->bodypartsList();

	for (int cam = 0; cam < m_numCameras; cam++) {
		primaryCombo->addItem(Dataset::dataset->cameraName(cam));
	}

	loadPaths();
	stackedWidget->setCurrentWidget(calibrationSetup);
	for (const auto& entity : Dataset::dataset->entitiesList()) {
		m_reprojectionErrors[entity] = new std::vector<double>(Dataset::dataset->bodypartsList().size());
		m_boneLengthErrors[entity] = new std::vector<double>(Dataset::dataset->skeleton().size());
	}

	switchToggledSlot(false);

	emit datasetLoaded();
}


void ReprojectionWidget::switchToggledSlot(bool toggle) {
	if (toggle) {
		if (Dataset::dataset == nullptr) {
			toggleSwitch->setToggled(false);
			return;
		}
		m_reprojectionActive = true;
		calculateAllReprojections();
		calculateReprojectionSlot(m_currentImgSetIndex, m_currentFrameIndex);
	}
	else {
		m_reprojectionActive = false;
		undoReprojection();
	}
	emit reprojectedPoints(Dataset::dataset->imgSets()[m_currentImgSetIndex], m_currentFrameIndex);
	emit reprojectionToolToggled(toggle);
}


void ReprojectionWidget::intrinsicsPathClickedSlot() {
	QString path = QFileDialog::getExistingDirectory(this, "Select Intrinsics Parameter Path", m_parameterDir.path());
	if (path != "") {
		m_parameterDir.setPath(path);
		m_parameterDir.cdUp();
		intrinsicsPathEdit->setText(path);
		m_intrinsicsPathValid = true;
	}
}


void ReprojectionWidget::extrinsicsPathClickedSlot() {
	QString path = QFileDialog::getExistingDirectory(this, "Select Extrnsics Parameter Path", m_parameterDir.path());
	if (path != "") {
		m_parameterDir.setPath(path);
		m_parameterDir.cdUp();
		extrinsicsPathEdit->setText(path);
		m_extrinsicsPathValid = true;
	}
}


bool ReprojectionWidget::checkIntrinsicsPath(QString path) {
	for (int cam = 0; cam < m_numCameras; cam++) {
		if (!QFile::exists(path + "/" + "Intrinsics_" + Dataset::dataset->cameraName(cam) + ".yaml")) {
			QErrorMessage *msg = new QErrorMessage();
			msg->showMessage("Intrinsics File \"Intrinsics_" + Dataset::dataset->cameraName(cam) + ".yaml\" does not exist.");
			return false;
		}
	}
	return true;
}


bool ReprojectionWidget::checkExtrinsicsPath(QString path) {
	for (int cam = 0; cam < m_numCameras; cam++) {
		if (cam != primaryCombo->currentIndex() && !QFile::exists(path + "/" + "Extrinsics_" + primaryCombo->currentText() + "_" + Dataset::dataset->cameraName(cam) + ".yaml")) {
			QErrorMessage *msg = new QErrorMessage();
			msg->showMessage("Extrinsics File \"Extrinsics_" + primaryCombo->currentText() + "_" + Dataset::dataset->cameraName(cam) + ".yaml\" does not exist.");
			return false;
		}
	}
	return true;
}


void ReprojectionWidget::initReprojectionClickedSlot() {
	if (!checkIntrinsicsPath(intrinsicsPathEdit->text()) || !checkExtrinsicsPath(extrinsicsPathEdit->text())) {
		return;
	}
	getSettings();
	savePaths();
	toggleSwitch->setEnabled(true);
	toggleSwitch->setToggled(true);
	emit reprojectionToolToggled(true);
	m_reprojectionActive = true;
	QList<QString> intrinsicsList;
	for (int cam = 0; cam < m_numCameras; cam++) {
		intrinsicsList.append(intrinsicsPathEdit->text() + "/" + "Intrinsics_" + Dataset::dataset->cameraName(cam) + ".yaml");
	}
	QList<QString> extrinsicsList;
	for (int cam = 0; cam < m_numCameras; cam++) {
			extrinsicsList.append(extrinsicsPathEdit->text() + "/" + "Extrinsics_" + primaryCombo->currentText() +"_" + Dataset::dataset->cameraName(cam) + ".yaml");
	}
	reprojectionTool = new ReprojectionTool(intrinsicsList, extrinsicsList,primaryCombo->currentIndex());
	stackedWidget->setCurrentWidget(reprojectionChartWidget);
	modeLabel->show();
	modeCombo->show();
	calculateAllReprojections();
	calculateReprojectionSlot(m_currentImgSetIndex, m_currentFrameIndex);
}


void ReprojectionWidget::savePaths() {
	settings->beginGroup("CalibrationPaths");
	settings->setValue("NumberCameras", Dataset::dataset->numCameras());
	settings->setValue("IntrinsicsPath", intrinsicsPathEdit->text());
	settings->setValue("ExtrinsicsPath", extrinsicsPathEdit->text());
	settings->setValue("PrimaryIndex", primaryCombo->currentIndex());
	settings->endGroup();
}


void ReprojectionWidget::loadPaths() {
	settings->beginGroup("CalibrationPaths");
	if(settings->value("NumberCameras").toInt() == Dataset::dataset->numCameras()) {
			intrinsicsPathEdit->setText(settings->value("IntrinsicsPath").toString());
			extrinsicsPathEdit->setText(settings->value("ExtrinsicsPath").toString());
			primaryCombo->setCurrentIndex(settings->value("PrimaryIndex").toInt());
			if (settings->contains("ExtrinsicsPath")) {
				m_parameterDir.setPath(settings->value("ExtrinsicsPath").toString());
				m_parameterDir.cdUp();
			}
			else {
				m_parameterDir.setPath(QDir::homePath());
			}

		}
	settings->endGroup();
}


void ReprojectionWidget::calculateReprojectionSlot(int currentImgSetIndex, int currentFrameIndex) {
	m_currentImgSetIndex = currentImgSetIndex;
	m_currentFrameIndex = currentFrameIndex;
	QMap<QString, cv::Mat> reconPointsMap;
	if (m_reprojectionActive) {
		for (const auto& entity : m_entitiesList) {
			for (const auto& bodypart : m_bodypartsList) {
				QList<int> camsToUse;
				QList<int> alreadyReprojected;
				QList<QPointF> points;
				int camCounter = 0;
				for (const auto& frame : Dataset::dataset->imgSets()[currentImgSetIndex]->frames) {
					if (frame->keypointMap[entity + "/" + bodypart]->state() == Annotated) {
						camsToUse.append(camCounter);
						points.append(frame->keypointMap[entity + "/" + bodypart]->coordinates());
					}
					else if (frame->keypointMap[entity + "/" + bodypart]->state() == Reprojected) {
						alreadyReprojected.append(camCounter);
					}
					camCounter++;
				}
				if (camsToUse.size() >= m_minViews) {
					cv::Mat X =  reprojectionTool->reconstructPoint3D(points, camsToUse);
					reconPointsMap[entity + "/" + bodypart] = X;
					QList<QPointF> reprojectedPoints = reprojectionTool->reprojectPoint(X);
					double reprojectionError = 0;
					for (int cam = 0; cam < reprojectedPoints.size(); cam ++) {
						QSize imgSize = Dataset::dataset->imgSets()[currentImgSetIndex]->frames[cam]->imageDimensions;
						QRectF imgRect(QPoint(0,0), imgSize);
						Keypoint *keypoint = Dataset::dataset->imgSets()[currentImgSetIndex]->frames[cam]->keypointMap[entity + "/" + bodypart];
						if (!camsToUse.contains(cam) && imgRect.contains(reprojectedPoints[cam])) {
							if (keypoint->state() != Suppressed) {
								keypoint->setState(Reprojected);
								keypoint->setCoordinates(reprojectedPoints[cam]);
							}
						}
						else if (keypoint->state() == Annotated) {
							QPointF annotatedPoint = Dataset::dataset->imgSets()[currentImgSetIndex]->frames[cam]->keypointMap[entity + "/" + bodypart]->coordinates();
							QPointF dist = annotatedPoint-reprojectedPoints[cam];
							reprojectionError += sqrt(dist.x()*dist.x()+dist.y()*dist.y())/reprojectedPoints.size();
						}
					}
						(*m_reprojectionErrors[entity])[Dataset::dataset->bodypartsList().indexOf(bodypart)] = reprojectionError;
				}
				else {
					(*m_reprojectionErrors[entity])[Dataset::dataset->bodypartsList().indexOf(bodypart)] = 0;
					for (int cam = 0; cam < Dataset::dataset->numCameras(); cam ++) {
						Keypoint *keypoint = Dataset::dataset->imgSets()[currentImgSetIndex]->frames[cam]->keypointMap[entity + "/" + bodypart];
						if (keypoint->state() == Reprojected) {
							keypoint->setState(NotAnnotated);
						}
					}
				}
			}
		}
		for (const auto& entity : m_entitiesList) {
			int idx = 0;
			for (const auto& comp : Dataset::dataset->skeleton()) {
				if (reconPointsMap.contains(entity + "/" + comp.keypointA) && reconPointsMap.contains(entity + "/" + comp.keypointB)) {
					double dist = cv::norm(reconPointsMap[entity + "/" + comp.keypointA] - reconPointsMap[entity + "/" +  comp.keypointB]);
					(*m_boneLengthErrors[entity])[idx++] = dist - comp.length;
				}
				else {
					(*m_boneLengthErrors[entity])[idx++] = 0.0;
				}
			}
		}
		emit reprojectedPoints(Dataset::dataset->imgSets()[currentImgSetIndex], currentFrameIndex);
		emit reprojectionToolToggled(true);
		reprojectionChartWidget->reprojectionErrorsUpdatedSlot(m_reprojectionErrors);
		boneLengthChartWidget->boneLengthErrorsUpdatedSlot(m_boneLengthErrors);
	}
}


void ReprojectionWidget::calculateAllReprojections() {
	if (m_reprojectionActive) {
		for (const auto& imgSet : Dataset::dataset->imgSets()) {
			for (const auto& entity : m_entitiesList) {
				for (const auto& bodypart : m_bodypartsList) {
					QList<int> camsToUse;
					QList<int> alreadyReprojected;
					QList<QPointF> points;
					int camCounter = 0;
					for (const auto& frame : imgSet->frames) {
						if (frame->keypointMap[entity + "/" + bodypart]->state() == Annotated) {
							camsToUse.append(camCounter);
							points.append(frame->keypointMap[entity + "/" + bodypart]->coordinates());
						}
						else if (frame->keypointMap[entity + "/" + bodypart]->state() == Reprojected) {
							alreadyReprojected.append(camCounter);
						}
						camCounter++;
					}
					if (camsToUse.size() >= m_minViews) {
						cv::Mat X =  reprojectionTool->reconstructPoint3D(points, camsToUse);
						QList<QPointF> reprojectedPoints = reprojectionTool->reprojectPoint(X);
						double reprojectionError = 0;
						for (int cam = 0; cam < reprojectedPoints.size(); cam ++) {
							QSize imgSize =imgSet->frames[cam]->imageDimensions;
							QRectF imgRect(QPoint(0,0), imgSize);
							Keypoint *keypoint = imgSet->frames[cam]->keypointMap[entity + "/" + bodypart];
							if (!camsToUse.contains(cam) && imgRect.contains(reprojectedPoints[cam])) {
								if (keypoint->state() != Suppressed) {
									keypoint->setState(Reprojected);
									keypoint->setCoordinates(reprojectedPoints[cam]);
								}
							}
							else if (keypoint->state() == Annotated) {
								QPointF annotatedPoint = imgSet->frames[cam]->keypointMap[entity + "/" + bodypart]->coordinates();
								QPointF dist = annotatedPoint-reprojectedPoints[cam];
								reprojectionError += sqrt(dist.x()*dist.x()+dist.y()*dist.y())/reprojectedPoints.size();
							}
						}
							(*m_reprojectionErrors[entity])[Dataset::dataset->bodypartsList().indexOf(bodypart)] = reprojectionError;
					}
					else {
						(*m_reprojectionErrors[entity])[Dataset::dataset->bodypartsList().indexOf(bodypart)] = 0;
						for (int cam = 0; cam < Dataset::dataset->numCameras(); cam ++) {
							Keypoint *keypoint = imgSet->frames[cam]->keypointMap[entity + "/" + bodypart];
							if (keypoint->state() == Reprojected) {
								keypoint->setState(NotAnnotated);
							}
						}
					}
				}
			}
		}
	}
}


void ReprojectionWidget::undoReprojection() {
	for (auto& imgSet : Dataset::dataset->imgSets()) {
		for (auto& frame : imgSet->frames) {
			for (auto& keypoint : frame->keypoints) {
				if (keypoint->state() == Reprojected) keypoint->setState(NotAnnotated);
			}
		}
	}
}


void ReprojectionWidget::getSettings() {
	settings->beginGroup("Settings");
	settings->beginGroup("ReprojectionSettings");
	if (settings->contains("MinViews")) {
		m_minViews = settings->value("MinViews").toInt();
	}
	minViewsChangedSlot(m_minViews);
	// if (settings->contains("errorThreshold")) {
	// 	m_errorThreshold = settings->value("errorThreshold").toDouble();
	// }
	// errorThresholdChangedSlot(m_errorThreshold);
	settings->endGroup();
	settings->endGroup();
}


void ReprojectionWidget::minViewsChangedSlot(int value) {
	m_minViews = value;
	calculateAllReprojections();
	calculateReprojectionSlot(m_currentImgSetIndex, m_currentFrameIndex);
}


// void ReprojectionWidget::errorThresholdChangedSlot(double value) {
// 	m_errorThreshold = value;
// 	emit errorThresholdChanged(value);
// }

void ReprojectionWidget::modeComboChangedSlot(const QString& mode) {
	if (mode == "Reprojection") {
		stackedWidget->setCurrentWidget(reprojectionChartWidget);
	}
	else {
		stackedWidget->setCurrentWidget(boneLengthChartWidget);

	}
}
