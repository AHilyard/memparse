/**
 * @file main.cpp
 *
 * @date 4/22/2016
 * @author Anthony Hilyard
 * @brief
 */
#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QMap>
#include <QDebug>

QMap<QString, qlonglong> units;

long calculateUsageFromRange(QString range)
{
	QStringList tokens = range.split("-");
	if (tokens.count() == 2)
	{
		bool okStart, okEnd;
		qlonglong total = tokens.at(1).toLongLong(&okEnd, 16) - tokens.at(0).toLongLong(&okStart, 16);
		if (okStart && okEnd)
		{
			return total;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

QString humanFileSize(qlonglong bytes)
{
	QString final = QString::number(bytes);

	if (bytes > 1024 * 1024)
	{
		final += " (" + QString::number((double)bytes / (1024.0 * 1024.0), 'f', 1) + "M)";
	}
	else if (bytes > 1024)
	{
		final += " (" + QString::number((double) bytes / 1024.0, 'f', 1) + "K)";
	}

	return final;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (a.arguments().count() > 1)
	{
		QString pid = a.arguments().at(1);

		QFile mmapFile("/proc/" + pid + "/maps");

		if (mmapFile.open(QFile::ReadOnly))
		{
			QStringList mmapLines = QString(mmapFile.readAll()).split("\n");
			qDebug() << mmapLines.count();
			foreach (QString thisLine, mmapLines)
			{
				thisLine = thisLine.simplified();
				QStringList tokens = thisLine.split(" ");

				QString name;

				if (tokens.count() >= 6)
				{
					name = tokens.at(5);
				}
				else
				{
					name = "[anonymous]";
				}

				if (units.contains(name))
				{
					units[name] += calculateUsageFromRange(tokens.at(0));
				}
				else
				{
					units.insert(name, calculateUsageFromRange(tokens.at(0)));
				}
			}

			qlonglong totalBytes = 0;
			qlonglong sharedBytes = 0;
			foreach (QString thisUnit, units.keys())
			{
				qDebug() << qPrintable(thisUnit) << "\t" << qPrintable(humanFileSize(units[thisUnit]));
				totalBytes += units[thisUnit];
				if (thisUnit.contains(".so"))
				{
					sharedBytes += units[thisUnit];
				}
			}
			qDebug() << "Total unique\t" << qPrintable(humanFileSize(totalBytes - sharedBytes));
			qDebug() << "Total shared\t" << qPrintable(humanFileSize(sharedBytes));
			qDebug() << "Total\t\t" << qPrintable(humanFileSize(totalBytes));
		}
		else
		{
			qDebug() << "Unable to open maps file: " + mmapFile.fileName();
		}

	}
	else
	{
		qDebug("\nUsage: memparse PID\n\nDisplay memory usage info about a running process\n");
	}

	return 0;
}
