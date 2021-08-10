/*------------------------------------------------------------
 *  datasetcreator.hpp
 *  Created: 23. October 2021
 *  Author:   Timo Hüser
 *------------------------------------------------------------*/

#ifndef DATASETCREATOR_H
#define DATASETCREATOR_H

#include "globals.hpp"
#include "imagewriter.hpp"
#include "videostreamer.hpp"

#include "opencv2/videoio/videoio.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <QRunnable>

Q_DECLARE_METATYPE(QList<cv::Mat>)


class DatasetCreator : public QObject {
	Q_OBJECT
	public:
		explicit DatasetCreator(DatasetConfig *datasetConfig);

		signals:
			void gotAllDCTs();
			void datasetCreated();
			void datasetCreationFailed(QString errorMsg);
			void recordingBeingProcessedChanged(QString recording, QList<QString> cameras);
			void currentSegmentChanged(QString segmentName);
			void dctProgress(int index, int windowSizeint, int threadNumber);
			void startedClustering();
			void finishedClustering();
			void copyImagesStatus(int frameCount, int totalNumFrames, int threadNumber);
			void creationCanceled();

		public slots:
			void createDatasetSlot(QList<RecordingItem> recordings, QList<QString> entities, QList<QString> keypoints);
			void cancelCreationSlot();

	private:
		DatasetConfig *m_datasetConfig;
		QList<RecordingItem> m_recordingItems;
		QList<QString> m_entitiesList;
		QList<QString> m_keypointsList;
		QMap<int,QList<cv::Mat>> m_dctMap;
		QMap<int,int> m_frameNumberMap;
		bool m_creationCanceled = false;

		QList<QString> getCameraNames(const QString & path);
		QString getVideoFormat(const QString& recording);
		bool checkFrameCounts(const QString& recording, QList<QString> cameras);
		QList<int> extractFrames(const QString &path, QList<TimeLineWindow> timeLineWindows, QList<QString> cameras);
		QList<QString> getAndCopyFrames(const QString& recording, QList<QString> cameras, const QString& dataFolder, QList<int> frameNumbers);
		void createSavefile(const QString& recording, QList<QString> cameraNames, const QString& dataFolder, QList<int> frameNumbers);
		QMap<QString, QList<TimeLineWindow>> getRecordingSubsets(QList<TimeLineWindow> timeLineWindows);


	private slots:
		void computedDCTsSlot(QList<cv::Mat> dctImages, QMap<int,int> frameNumberMap, int threadNumber);

};

#endif