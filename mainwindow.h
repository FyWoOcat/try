#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QCryptographicHash>

#include <QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QBitArray>
#include <QDataStream>
#include <QFile>
#include <QDebug>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // 哈夫曼树结点结构
    struct HTNode {
        int weight; // 权值
        int parent; // 双亲结点下标
        int lchild; // 左孩子结点下标
        int rchild; // 右孩子结点下标
    };

    // 哈夫曼编码结构
    struct HTCode {
        QString code; // 编码
        int length; // 编码长度
    };
    void buildHuffmanTree(QVector<HTNode> &nodes, const QMap<quint8, int> &freqs);
    void generateHuffmanCode(const QVector<HTNode> &nodes, QVector<HTCode> &codes);
    void compressFile(const QString &inputFile, const QString &outputFile);
    void decompressFile(const QString &inputFile, const QString &outputFile);
    QByteArray bitsToBytes(const QBitArray &bits);
    QBitArray bytesToBits(const QByteArray &bytes);
private slots:
    void on_toZip_clicked();

    void on_Zipto_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
