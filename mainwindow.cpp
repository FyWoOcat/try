#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildHuffmanTree(QVector<HTNode> &nodes, const QMap<quint8, int> &freqs) {
    // 初始化结点
    int n = freqs.size(); // 字节种类数
    nodes.resize(2 * n - 1); // 结点总数
    int i = 0;
    for (auto it = freqs.begin(); it != freqs.end(); ++it) {
        nodes[i].weight = it.value(); // 权值为出现次数
        nodes[i].parent = -1; // 初始时无双亲
        nodes[i].lchild = -1; // 初始时无左孩子
        nodes[i].rchild = -1; // 初始时无右孩子
        i++;
    }
    // 构建树
    for (int k = n; k < 2 * n - 1; ++k) {
        // 选择两个权值最小且无双亲的结点
        int min1 = INT_MAX, min2 = INT_MAX;
        int s1 = -1, s2 = -1;
        for (int j = 0; j < k; ++j) {
            if (nodes[j].parent == -1) {
                if (nodes[j].weight < min1) {
                    min2 = min1;
                    s2 = s1;
                    min1 = nodes[j].weight;
                    s1 = j;
                } else if (nodes[j].weight < min2) {
                    min2 = nodes[j].weight;
                    s2 = j;
                }
            }
        }
        // 合并两个结点为一个新的结点
        nodes[s1].parent = k;
        nodes[s2].parent = k;
        nodes[k].weight = nodes[s1].weight + nodes[s2].weight;
        nodes[k].parent = -1;
        nodes[k].lchild = s1;
        nodes[k].rchild = s2;
    }
}
void MainWindow::generateHuffmanCode(const QVector<HTNode> &nodes, QVector<HTCode> &codes) {
    // 初始化编码
    int n = nodes.size() / 2 + 1; // 字节种类数
    codes.resize(n);
    for (int i = 0; i < n; ++i) {
        codes[i].code = "";
        codes[i].length = 0;
    }
    // 从叶子结点开始向上回溯，生成编码
    for (int i = 0; i < n; ++i) {
        int cur = i; // 当前结点下标
        int p = nodes[cur].parent; // 双亲结点下标
        while (p != -1) {
            // 如果是左孩子，编码为0，如果是右孩子，编码为1
            if (nodes[p].lchild == cur) {
                codes[i].code = "0" + codes[i].code;
            } else {
                codes[i].code = "1" + codes[i].code;
            }
            codes[i].length++; // 编码长度加1
            cur = p; // 向上回溯
            p = nodes[cur].parent;
        }
    }
}
QByteArray MainWindow::bitsToBytes(const QBitArray &bits) {
    QByteArray bytes;
    bytes.fill(0);
    for (int i = 0; i < bits.size(); ++i) {
        // 如果是每个字节的第一位，就向字节数组添加一个新的字节
        if (i % 8 == 0) {
            bytes.append('\0');
        }
        // 根据位的值设置字节的相应位
        if (bits.testBit(i)) {
            bytes[i / 8] = bytes.at(i / 8) | (1 << (7 - i % 8));
        }
    }
    return bytes;
}
QBitArray MainWindow::bytesToBits(const QByteArray &bytes) {
    QBitArray bits;
    bits.resize(bytes.size() * 8);
    for (int i = 0; i < bytes.size(); ++i) {
        for (int j = 0; j < 8; ++j) {
            bits.setBit(i * 8 + j, bytes.at(i) & (1 << (7 - j)));
        }
    }
    return bits;
}
void MainWindow::compressFile(const QString &inputFile, const QString &outputFile) {
    // 检查输入文件的路径是否有效
    if (inputFile.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Input file path is empty");
        return;
    }
    QFile in(inputFile);
    if (!in.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Cannot open input file");
        return;
    }
    // 读取输入文件的数据
    QByteArray data = in.readAll();
    in.close();
    // 检查输入文件的数据是否为空
    if (data.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Input file is empty");
        return;
    }
    // 统计每个字节出现的次数
    QMap<quint8, int> freqs;
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = data[i];
        freqs[byte]++;
    }
    // 检查输入文件的字符频率是否相同
    if (freqs.size() == 1) {
        QMessageBox::critical(nullptr, "Error", "Input file has only one character");
        return;
    }
    // 构建哈夫曼树
    QVector<HTNode> nodes;
    buildHuffmanTree(nodes, freqs);
    // 生成哈夫曼编码
    QVector<HTCode> codes;
    generateHuffmanCode(nodes, codes);
    // 检查输出文件的路径是否有效
    if (outputFile.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Output file path is empty");
        return;
    }
    QFile out(outputFile);
    if (!out.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Cannot open output file");
        return;
    }
    // 写入文件头
    QDataStream ds(&out);
    ds.writeBytes("HUFFMAN", 7); // 写入一个带长度的字节数组
    // 写入文件的原始后缀名
    QFileInfo fi(inputFile); // 获取文件信息
    QString suffix = fi.suffix(); // 获取文件后缀名
    // 检查文件的原始后缀名是否为空
    if (suffix.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Input file has no suffix");
        return;
    }
    // 检查文件的原始后缀名是否过长，如果过长，截断为4个字符
    if (suffix.size() > 4) {
        suffix = suffix.left(4);
    }
    ds.writeBytes(suffix.toUtf8().data(), suffix.toUtf8().size()); // 写入一个带长度的字节数组
    // 写入哈夫曼树的结构
    ds << nodes.size(); // 结点总数
    for (int i = 0; i < nodes.size(); ++i) {
        ds << nodes[i].weight << nodes[i].parent << nodes[i].lchild << nodes[i].rchild;
    }
    // 写入压缩后的数据
    QBitArray bits; // 用位数组存储压缩后的数据
    int bitIndex = 0; // 当前位的下标
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = data[i];
        HTCode code = codes[byte]; // 获取对应的哈夫曼编码
        for (int j = 0; j < code.length; ++j) {
            // 扩展位数组的大小
            if (bitIndex == bits.size()) {
                bits.resize(bitIndex + 1);
            }
            // 根据编码设置位的值
            if (code.code[j] == '0') {
                bits[bitIndex] = false;
            } else {
                bits[bitIndex] = true;
            }
            bitIndex++;
        }
    }
    // 将位数组转化为字节数组
    QByteArray compressed = bitsToBytes(bits);
    // 检查压缩后的数据是否为空
    if (compressed.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Compression error: no data generated");
        return;
    }
    // 检查压缩后的数据是否比原始数据还大，如果是，放弃压缩，直接写入原始数据
    if (compressed.size() >= data.size()) {
        QMessageBox::warning(nullptr, "Warning", "Compression ratio is greater than or equal to 1. No compression is performed.");
        compressed = data;
    }
    // 写入校验和
    QByteArray checksum = QCryptographicHash::hash(compressed, QCryptographicHash::Md5); // 计算压缩后的数据的MD5哈希值
    ds.writeBytes(checksum.data(), checksum.size()); // 写入一个带长度的字节数组
    // 写入压缩后的数据
    ds << compressed;
    // 写入文件尾
    ds.writeBytes("END", 3); // 写入一个带长度的字节数组
    // 写入文件大小
    ds << out.size(); // 写入一个整数
    // 检查输出文件的写入是否成功，如果失败，删除输出文件
    if (ds.status() != QDataStream::Ok) {
        QMessageBox::critical(nullptr, "Error", "Output file write error");
        out.close();
        QFile::remove(outputFile);
        return;
    }
    out.close();
    // 输出压缩信息
    qDebug() << "Input file size:" << data.size() << "bytes";
    qDebug() << "Output file size:" << out.size() << "bytes";
    qDebug() << "Compression ratio:" << (double)out.size() / data.size();
}
void MainWindow::decompressFile(const QString &inputFile, const QString &outputFile) {
    // 打开输入文件
    QFile in(inputFile);
    if (!in.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Cannot open input file");
        return;
    }
    // 获取输入文件的大小
    qint64 inSize = in.size();
    if (inSize == 0) {
        QMessageBox::critical(nullptr, "Error", "Input file is empty");
        return;
    }
    // 读取输入文件的数据
    QByteArray inData = in.readAll(); // 读取整个文件
    in.close();
    QDataStream ds(inData); // 使用 QByteArray 来创建数据流
    // 读取文件头
    QByteArray header; // 文件头的字节数组
    ds >> header; // 读取一个带长度的字节数组
    QString headerStr = QString::fromUtf8(header); // 使用 UTF-8 编码来转化为字符串
    if (headerStr != "HUFFMAN") {
        QMessageBox::critical(nullptr, "Error", "Invalid file format: not a Huffman compressed file");
        return;
    }
    // 读取文件的原始后缀名
    QByteArray suffix; // 后缀名的字节数组
    ds >> suffix; // 读取一个带长度的字节数组
    QString suffixStr = QString::fromUtf8(suffix); // 使用 UTF-8 编码来转化为字符串
    // 读取哈夫曼树的结构
    int nodeCount; // 结点总数
    ds >> nodeCount;
    QVector<HTNode> nodes(nodeCount); // 结点数组
    for (int i = 0; i < nodeCount; ++i) {
        ds >> nodes[i].weight >> nodes[i].parent >> nodes[i].lchild >> nodes[i].rchild;
    }
    // 读取校验和
    QByteArray checksum; // 校验和的字节数组
    ds >> checksum; // 读取一个带长度的字节数组
    // 读取压缩后的数据
    QByteArray compressed; // 压缩后的字节数组
    ds >> compressed;
    // 验证校验和
    QByteArray computed = QCryptographicHash::hash(compressed, QCryptographicHash::Md5); // 计算压缩后的数据的MD5哈希值
    if (checksum != computed) {
        QMessageBox::critical(nullptr, "Error", "Decompression error: checksum mismatch");
        return;
    }
    // 去掉多余的0
    int bitCount = nodeCount - 1; // 压缩后的数据的位数
    if (bitCount % 8 != 0) {
        compressed.chop(1); // 删除最后一个字节
    }
    // 将字节数组转化为位数组
    QBitArray bits = bytesToBits(compressed);
    // 读取文件尾
    QByteArray tail; // 文件尾的字节数组
    ds >> tail; // 读取一个带长度的字节数组
    QString tailStr = QString::fromUtf8(tail); // 使用 UTF-8 编码来转化为字符串
    if (tailStr != "END") {
        QMessageBox::critical(nullptr, "Error", "Decompression error: file tail not found");
        return;
    }
    // 读取文件大小
    qint64 outSize; // 文件大小
    ds >> outSize; // 读取一个整数
    if (outSize != inSize) {
        QMessageBox::critical(nullptr, "Error", "Decompression error: file size mismatch");
        return;
    }
    // 打开输出文件
    QFile out(outputFile);
    if (!out.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Cannot open output file");
        return;
    }
    // 解压数据
    QByteArray data; // 解压后的数据
    int root = nodeCount - 1; // 哈夫曼树的根结点下标
    int cur = root; // 当前结点下标
    // 预先分配足够的内存空间
    data.resize(inSize - header.size() - suffix.size() - sizeof(int) - sizeof(HTNode) * nodeCount - checksum.size() - sizeof(int) - compressed.size() - tail.size() - sizeof(qint64));
    // 获取指向数据的指针
    char *dataPtr = data.data();
    // 记录当前写入的位置
    int pos = 0;
    for (int i = 0; i < bits.size(); ++i) {
        // 根据位的值选择左孩子或右孩子
        if (bits[i] == false) {
            cur = nodes[cur].lchild;
        } else {
            cur = nodes[cur].rchild;
        }
        // 如果到达叶子结点，输出对应的字节
        if (nodes[cur].lchild == -1 && nodes[cur].rchild == -1) {
            // 使用 std::copy 函数来复制数据
            std::copy(reinterpret_cast<const char *>(&cur), reinterpret_cast<const char *>(&cur) + sizeof(quint8), dataPtr + pos);
            pos += sizeof(quint8);
            cur = root; // 重新回到根结点
        }
        // 检查是否超过了哈夫曼树的大小
        if (cur >= nodeCount) {
            QMessageBox::critical(nullptr, "Error", "Decompression error: bit array size exceeds Huffman tree size");
            return;
        }
    }
    // 写入输出文件
    out.write(data); // 使用 QFile 的 write 方法来写入数据
    out.close();
    // 将输出文件的后缀名改为原始后缀名
    QFileInfo fi(out);
    QString newName = fi.absolutePath() + "/" + fi.baseName() + "." + suffixStr;
    out.rename(newName);


}
void MainWindow::on_toZip_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(nullptr, "Select input file");
    if (inputFile.isEmpty()) {
        return;
    }
    // 获取输出文件的路径
    QString outputFile = QFileDialog::getSaveFileName(nullptr, "Select output file", "", "*.Fycat");
    if (outputFile.isEmpty()) {
        return;
    }
    compressFile(inputFile, outputFile);
}


void MainWindow::on_Zipto_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(nullptr, "Select input file", "", "*.Fycat");
    if (inputFile.isEmpty()) {
        return;
    }
    // 获取输出文件的路径
    QString outputFile = QFileDialog::getSaveFileName(nullptr, "Select output file");
    if (outputFile.isEmpty()) {
        return;
    }
    // 解压文件
    decompressFile(inputFile, outputFile);
}

