#pragma execution_character_set("utf-8")
#include "CCompiler.h"

//输出信息
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
    setWindowTitle(tr("C-Compiler 2051496 秦睿洋"));
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
        QMessageBox::information(this, "错误", "文件不可读!");
        return;
    }
    qDebug() << "Yes read";
    QTextStream in(&file);
    QString input = in.readAll();//读取所有
    ui.CodeTextEdit->setPlainText(input);
    file.close();
    ui.StartButton->setEnabled(true);
}

void CCompiler::startAnalysis() {
    outputString.clear();
    outputString += "词法分析结果:\n";

    Lexer lex(this->fileName.toLocal8Bit().constData());
    lex.analyse();
    lex.output();

    if (errorFlag) {
        outputString = errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "错误", errorString.c_str());
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
        QMessageBox::information(this, "错误", errorString.c_str());
        return;
    }

    outputString += "\n\n==============================================\n";
    outputString += "语法分析过程:\n";

    gs.analyse(lex.getResult());

    if (errorFlag) {
        outputString += errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "错误", errorString.c_str());
        return;
    }

    outputString += "\n\n==============================================\n";
    outputString += "中间代码:\n";
    gs.outputIntermediateCode();
    
    IntermediateCode* ic = gs.getIntermediateCode();
    ic->divideBlocks(gs.getFuncEnter());

    outputString += "\n\n==============================================\n";
    outputString += "代码块划分:\n";
    ic->outputBlocks();


    outputString += "\n\n==============================================\n";
    outputString += "活跃/待用信息表:\n";

    ObjectCode oc;
    oc.analyseBlock(ic->getBlock());
    oc.outputIBlocks();

    outputString += "\n\n==============================================\n";
    outputString += "目标代码:\n";

    oc.generateCode();
    oc.outputObjectCode();
    oc.outputObjectCode("test.asm");

    if (errorFlag) {
        outputString += errorString;
        QString outpout = QString::fromStdString(outputString);
        ui.ResultTextEdit->setPlainText(outpout);
        QMessageBox::information(this, "错误", errorString.c_str());
        return;
    }

    
    QString outpout = QString::fromStdString(outputString);
    ui.ResultTextEdit->setPlainText(outpout);
    fclose(stdin);
}
