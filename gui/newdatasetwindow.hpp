/*------------------------------------------------------------
 *  newdatasetwindow.hpp
 *  Created: 07. July 2020
 *  Author:   Timo Hüser
 * Contact: 	timo.hueser@gmail.com
 *------------------------------------------------------------*/

#ifndef NEWDATASETWINDOW_H
#define NEWDATASETWINDOW_H

#include "globals.hpp"
#include "dataset.hpp"
#include "presetswindow.hpp"
#include "configurableitemlist.hpp"
#include "datasetcreator.hpp"
#include "videocutterwindow.hpp"



#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QSettings>
#include <QToolBar>
#include <QListWidget>
#include <QRadioButton>
#include <QSpinBox>
#include <QComboBox>
#include <QErrorMessage>


class NewDatasetWindow : public QWidget {
	Q_OBJECT
	public:
		explicit NewDatasetWindow(QWidget *parent = nullptr);

	public slots:

	signals:
		void createDataset(QList<QString> recordings, QList<QString> entities, QList<QString> keypoints);

	private:
		VideoCutterWindow *videoCutterWindow;

		DatasetConfig	*m_datasetConfig;
		DatasetCreator *datasetCreator;
		QSettings *settings;
		PresetsWindow *loadPresetsWindow;
		PresetsWindow *savePresetsWindow;
		QList<QString> presets;

		QLineEdit *datasetNameEdit;
		QLineEdit *datasetPathEdit;
		QPushButton *datasetPathButton;
		QRadioButton *videosButton;
		QRadioButton *imagesButton;
		QSpinBox *numCamerasBox;
		QSpinBox *frameSetsRecordingBox;
		QComboBox *samplingMethodCombo;

		ConfigurableItemList *recordingsItemList;
		ConfigurableItemList *entitiesItemList;
		ConfigurableItemList *keypointsItemList;

		QPushButton *loadButton;
		QPushButton *saveButton;
		QPushButton *createButton;

		QErrorMessage *m_errorMsg;



		private slots:
			void datasetNameChangedSlot(const QString &name);
			void datasetPathChangedSlot(const QString &path);
			void datasetPathClickedSlot();
			void dataTypeChangedSlot();
			void numCamerasChangedSlot(int num);
			void frameSetsRecordingChandedSlot(int num);
			void samplingMethodChangedSlot(const QString &method);

			void createDatasetClickedSlot();
			void datasetCreatedSot();
			void datasetCreationFailedSlot(QString errorMsg);


			void savePresetsClickedSlot();
			void loadPresetsClickedSlot();
			void savePresetSlot(const QString& preset);
			void loadPresetSlot(const QString& preset);
};


class LabelWithToolTip : public QWidget {
	Q_OBJECT
	public:
		explicit LabelWithToolTip(QString name, QString toolTip, QWidget *parent = nullptr);
};


#endif