#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QPushButton>
#include <qmessagebox.h>
#include "ui_CCompiler.h"

#include <vector>
#include <string>
#include <Windows.h>
#include <fstream>
#include <map>
#include <filesystem>

#include "Global.h"
#include "Lexer.h"
#include "GrammerAndSemantic.h"
#include "ObjectCode.h"

class CCompiler : public QMainWindow
{
    Q_OBJECT

public:
    CCompiler(QWidget *parent = nullptr);
    ~CCompiler();

private:
    Ui::CCompilerClass ui;

    //�ļ���
    QString fileName;
   

    // ��ʼ��UI�����״̬
    void InitWindow();
    // ��ʼ��connection
    void InitConnections();
    
private slots:
    //��ȡ�ļ�
    void selectFile();
    //��ʼ����
    void startAnalysis();
};
