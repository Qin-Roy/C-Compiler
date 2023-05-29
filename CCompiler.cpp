#pragma execution_character_set("utf-8")
#include "CCompiler.h"

//�����Ϣ
std::string outputString;
bool errorFlag;
std::string errorString;


CCompiler::CCompiler(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    InitWindow();
    InitConnections();
}

CCompiler::~CCompiler()
{}

void CCompiler::InitWindow() {
    setWindowTitle(tr("C-Compiler 2051496 �����"));
    this->setMinimumSize(QSize(1083, 720));
    this->setMaximumSize(QSize(1083, 720));
    ui.SelectFileButton->setEnabled(true);
    ui.StartButton->setEnabled(false);
    ui.PathLineEdit->setReadOnly(true);
    ui.CodeTextEdit->setReadOnly(true);
    ui.ResultTextEdit->setReadOnly(true);
}

void CCompiler::InitConnections() {
    connect(ui.SelectFileButton, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
    connect(ui.StartButton, SIGNAL(clicked(bool)), this, SLOT(startAnalysis()));
}

void CCompiler::selectFile() {
    errorFlag = 0;
    errorString.clear();
    outputString.clear();

    this->fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "D:\\");
    ui.PathLineEdit->clear();
    ui.PathLineEdit->setText(this->fileName);

    QFile file(this->fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, "����", "�ļ����ɶ�!");
        return;
    }
    qDebug() << "Yes read";
    QTextStream in(&file);
    QString input = in.readAll();//��ȡ����
    ui.CodeTextEdit->setPlainText(input);
    file.close();
    ui.StartButton->setEnabled(true);
}

void CCompiler::startAnalysis() {
    outputString.clear();
    outputString += "�ʷ��������:\n";

    Lexer lex(this->fileName.toLocal8Bit().constData());
    lex.analyse();
    lex.output();

    if (errorFlag) {
        outputString = errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "����", errorString.c_str());
        return;
    }
   
    outputString += "\n\n==============================================\n";
    outputString += "DFA:\n";
    
    GrammerAndSemantic gs("grammer.txt");
    gs.outputDFA();

    if (errorFlag) {
        outputString += errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "����", errorString.c_str());
        return;
    }

    outputString += "\n\n==============================================\n";
    outputString += "�﷨��������:\n";

    gs.analyse(lex.getResult());

    if (errorFlag) {
        outputString += errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "����", errorString.c_str());
        return;
    }

    outputString += "\n\n==============================================\n";
    outputString += "�м����:\n";
    gs.outputIntermediateCode();
    
    IntermediateCode* ic = gs.getIntermediateCode();
    ic->divideBlocks(gs.getFuncEnter());

    outputString += "\n\n==============================================\n";
    outputString += "����黮��:\n";
    ic->outputBlocks();


    outputString += "\n\n==============================================\n";
    outputString += "��Ծ/������Ϣ��:\n";

    ObjectCode oc;
    oc.analyseBlock(ic->getBlock());
    oc.outputIBlocks();

    outputString += "\n\n==============================================\n";
    outputString += "Ŀ�����:\n";

    oc.generateCode();
    oc.outputObjectCode();
    oc.outputObjectCode("test.asm");

    if (errorFlag) {
        outputString += errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "����", errorString.c_str());
        return;
    }

    
    QString outpout = QString::fromStdString(outputString);
    ui.ResultTextEdit->setPlainText(outpout);
    fclose(stdin);
}
