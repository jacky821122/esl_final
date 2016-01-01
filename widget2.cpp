/**/#include "widget2.h"
#include "sliderwidget.h"
#include "detectcolor.h"
#include <iostream>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QApplication>

#include <QString>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QStandardPaths>
#include <QImageReader>


using namespace cv;
using namespace std;

extern QImage Mat2QImage(const Mat3b &src);
Mat QImage2Mat(const QImage &inImage);
void chromakey(const Mat, const Mat, Mat*);

extern double red_l, red_h;
extern double green_l, green_h;
extern double blue_l, blue_h;

int recount;

widget2::widget2(QWidget *parent): cap(0)
{
	/*----------Initializing Everything-----------*/
	swidget = new sliderwidget();
	subqlabel = new SubQLabel();
	showSelectColor = new QLabel(this);
	showRGB = new QLabel(this);
	showqbackimg = new QLabel(this);
	showcap = new QLabel(this);
	showqresult = new QLabel(this);
	recount = 0;

	/*----------Setup the Size of Images and Windows-----------*/
	capsize = QSize(532, 399);
	this -> resize(800, 400); // In pandaboard
	this -> setWindowTitle("Ariel");

	/*-----------------Setup Capture Image Label-----------------*/
	showcapture = new QLabel(this);
	QPoint tmpoint = QPoint(0, capsize.height() + 20);
	QRect tmprect = QRect(tmpoint, capsize);
	showcapture -> setGeometry(tmprect);

	/*-----------------Setup Timer-----------------*/
	timer = new QTimer(this);
	timer -> start(30);
	connect(timer, SIGNAL(timeout()), this, SLOT(camera_caping()));
	
	/*-----------------Setup Record Timer-----------------*/
	recTime = new QTimer(this);
	connect(recTime, SIGNAL(timeout()), this, SLOT(record()));

	/*-----------------Setup The Result Image Label-----------------*/
	QPoint point = QPoint(268, 0);
	QRect rect = QRect(point, capsize);
	showqresult -> setGeometry(rect);
	showqbackimg -> setGeometry(rect);

	/*-----------------Setup The Switch Button-----------------*/
	bt_pannel = new QPushButton(tr("&Color Select"), this);
	bt_pannel -> setGeometry(84, 40, 100, 40);
	connect(bt_pannel, SIGNAL(clicked()), this, SLOT(control_pannel_pop()));

	/*-----------------Setup The Capture Button-----------------*/
	bt_capture = new QPushButton(("&Capture"), this);
	bt_capture -> setGeometry(84, 110, 100, 40);
	connect(bt_capture, SIGNAL(clicked()), this, SLOT(capture()));

	/*-----------------Setup The Backing Image-----------------*/
	qbackimg = QImage("1new.jpg");
	bt_background = new QPushButton(tr("&Open"), this);
	bt_background -> setGeometry(84, 180, 100, 40);
	connect(bt_background, SIGNAL(clicked()), this, SLOT(open()));
	//qbackimg = QImage("1new.jpg");

	/*-----------------Setup Video Saving Button-----------------*/
	bt_record = new QPushButton(tr("&Record"), this);
	bt_record -> setGeometry(84, 250, 100, 40);
	connect(bt_record, SIGNAL(clicked()), this, SLOT(recording()));

	/*-----------------Setup Video Saving Stop Button-----------------*/
	bt_record_stop = new QPushButton(tr("&Stop"), this);
	bt_record_stop -> setGeometry(84, 250, 100, 40);
	bt_record_stop -> setHidden(true);
	connect(bt_record_stop, SIGNAL(clicked()), this, SLOT(stop_record()));

	/*-----------------Setup Image Button-----------------*/
	bt_saveimage = new QPushButton("&Save image", this);
	bt_saveimage -> setGeometry(84, 320, 100, 40);
	connect(bt_saveimage, SIGNAL(clicked()), this, SLOT(saveimage()));

	/*-----------------Setup The Connection between Slider and Mouse Detecting-----------------*/
	connect(subqlabel, SIGNAL(red_low_Changed( const int&)), swidget->red_low, SLOT(setValue(int)));
	connect(subqlabel, SIGNAL(red_high_Changed( const int&)), swidget->red_high, SLOT(setValue(int)));
	connect(subqlabel, SIGNAL(green_low_Changed( const int&)), swidget->green_low, SLOT(setValue(int)));
	connect(subqlabel, SIGNAL(green_high_Changed( const int&)), swidget->green_high, SLOT(setValue(int)));
	connect(subqlabel, SIGNAL(blue_low_Changed( const int&)), swidget->blue_low, SLOT(setValue(int)));
	connect(subqlabel, SIGNAL(blue_high_Changed( const int&)), swidget->blue_high, SLOT(setValue(int)));

	/*-----------------Setup Select Color Display-----------------*/
	showSelectColor -> setGeometry(0, 0, 800, 400);
	// showSelectColor -> setGeometry(0, 250, 150, 150);
	showRGB -> setGeometry(20, 5, 300, 20);
	showRGB -> setFont(QFont("Droid Sans Fallback", 15, QFont::Bold));
	
}


bool widget2::loadFile(const QString &fileName)
{
    	QImageReader reader(fileName);
    	reader.setAutoDetectImageFormat(true);
    	const QImage image = reader.read();
    	if (image.isNull()) {
        		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                      	tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        	setWindowFilePath(QString());
        	return false;
    	}

    	qbackimg = image;

    	return true;
}


void widget2::open()
{
    	QStringList mimeTypeFilters;
    	foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        	mimeTypeFilters.append(mimeTypeName);
    	mimeTypeFilters.sort();
    	//const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    	QFileDialog dialog(this, tr("Open File")/*, picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last()*/);
    	dialog.setAcceptMode(QFileDialog::AcceptOpen);
   	dialog.setMimeTypeFilters(mimeTypeFilters);
    	dialog.selectMimeTypeFilter("image/png");

    	while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void widget2::camera_caping()
{
	cap >> ccapimg; //CV_8UC3

	/*--------------For Testing---------------*/
	/*qcapimg = QImage("ahah2.png");
	qcapimg = qcapimg.convertToFormat(QImage::Format_RGB888);
	ccapimg = QImage2Mat(qcapimg);*/

	/*--------------Get Color Information---------------*/ 
	red_l = swidget -> red_low->sliderPosition();
	red_h = swidget -> red_high->sliderPosition();
	green_l = swidget -> green_low->sliderPosition();
	green_h = swidget -> green_high->sliderPosition();
	blue_l = swidget -> blue_low->sliderPosition();
	blue_h = swidget -> blue_high->sliderPosition();

	/*--------------Convert the BG to RGB888 then CV_8UC3---------------*/ 
	qbackimg = qbackimg.convertToFormat(QImage::Format_RGB888); //RGB888 (=) CV_8UC3
	cbackimg = QImage2Mat(qbackimg);

	/*-----------------Implement Chroma Key-----------------*/	
	chromakey(cbackimg, ccapimg, &cresult);
	
	/*-----------------Show the Result Image-----------------*/
	qresult = Mat2QImage(cresult);
	showqresult -> setPixmap(QPixmap::fromImage(qresult).scaled(capsize));

	/*-----------------Setup Select Color Display-----------------*/
	QPixmap *tmpix = new QPixmap(800, 400);
	tmpix -> fill(QColor::fromRgb(subqlabel->red, subqlabel->green, subqlabel->blue));
	showSelectColor -> setPixmap(tmpix->scaled(800, 400));
	QString str, str1, str2, str3;
	str1 = QString("RGB : ( ");
	str3 = str3.setNum(subqlabel->red);
	str3.append(",");
	str3.append(str2.setNum(subqlabel->green));
	str3.append(",");
	str3.append(str2.setNum(subqlabel->blue));
	str3.append(" )");
	str.append(str1);
	str.append(str3);
	showRGB -> setText(str);
}

void widget2::capture()
{
	subqlabel -> qcapimg = Mat2QImage(ccapimg);
	subqlabel -> setPixmap(QPixmap::fromImage(subqlabel->qcapimg).scaled(capsize));
  	subqlabel -> resize(capsize);

	QPoint pos = subqlabel->pos();
	QPoint pos1 = this->pos();
	if (pos.x() != pos1.x())
		pos.setX(pos1.x());
	if (pos.y() != pos1.y())
		pos.setY(pos1.y());
	subqlabel->move(pos);

  	subqlabel -> image_test = subqlabel -> qcapimg;
	subqlabel -> show();

}

void widget2::control_pannel_pop()
{
	swidget -> show();
}

void widget2::recording()
{
	recount = 1;
	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
	writer = VideoWriter ("MyVideo.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true);
	recTime -> start(30);
	bt_record -> setHidden(true);
	bt_record_stop -> setHidden(false);
}

void widget2::record()
{
	if (recount == 1) writer.write(cresult);
	else
	{
		recTime -> stop();
		bt_record_stop -> setHidden(true);
		bt_record -> setHidden(false);
	}
}

void widget2::saveimage(){

	vector<int> compression_params; 
	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); 
     	compression_params.push_back(98); 
     	imwrite("TestImage.jpg", cresult, compression_params); 
}

void widget2::stop_record()
{
	recount = 0;
}

void widget2::closeEvent(QCloseEvent *event)
{
	qApp -> quit();
	QWidget::closeEvent(event);
}

widget2::~widget2()
{}

void chromakey(const Mat cbackimg, const Mat ccapimg, Mat *dst){
	*dst = Mat(cbackimg.rows, cbackimg.cols, CV_8UC3);
	for(int y = 0 ; y < cbackimg.rows ; ++y)
	{
		for(int x = 0 ; x < cbackimg.cols ; ++x)
		{
			if(ccapimg.at<Vec3b>(y,x)[2] >= red_l && ccapimg.at<Vec3b>(y,x)[2] <= red_h && ccapimg.at<Vec3b>(y,x)[1] >= green_l && ccapimg.at<Vec3b>(y,x)[1] <= green_h && ccapimg.at<Vec3b>(y,x)[0] >= blue_l && ccapimg.at<Vec3b>(y,x)[0] <= blue_h)
			{
				dst->at<Vec3b>(y,x)[0] = cbackimg.at<Vec3b>(y,x)[0];
				dst->at<Vec3b>(y,x)[1] = cbackimg.at<Vec3b>(y,x)[1];
				dst->at<Vec3b>(y,x)[2] = cbackimg.at<Vec3b>(y,x)[2];
			}
			else
			{
				dst->at<Vec3b>(y,x)[0] = ccapimg.at<Vec3b>(y,x)[0];
				dst->at<Vec3b>(y,x)[1] = ccapimg.at<Vec3b>(y,x)[1];
				dst->at<Vec3b>(y,x)[2] = ccapimg.at<Vec3b>(y,x)[2];
			}
		}
	}
}

Mat QImage2Mat(const QImage &inImage)
{
	QImage   swapped = inImage.rgbSwapped(); 
	return Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
}
