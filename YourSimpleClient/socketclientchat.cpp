#include "socketclientchat.h"
#include "datablock.h"

using namespace YourSimpleClient;

SocketClientChat::SocketClientChat(QObject* parent) : QObject(parent)
{
    socketClient = new SocketClientCore(this);

    connect(socketClient, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(socketClient, SIGNAL(blockReceived(DataBlock &)), this, SLOT(onBlockReceived(DataBlock &)));

    socketClient->connectToHost(QHostAddress::LocalHost, 27016);

    if (!socketClient->waitForConnected(5000))
        qDebug() << "Connection wasn't established";

    socketClient->setId(QTime::currentTime().msecsSinceStartOfDay());
    socketClient->setName(QString("Client ").append(QString::number(socketClient->getId())));
    qDebug() << "My name for id" << socketClient->getId() << "is" << socketClient->getName();

    sendInfo();

    sendMessage("Hello, My Friend.");
}

SocketClientChat::~SocketClientChat()
{
    delete socketClient;
}

void SocketClientChat::onSocketDisconnected()
{
    qDebug() << "Disconnected from the server.";
}

void SocketClientChat::onBlockReceived(DataBlock &block)
{
    if (!block.receiver)
        return;

    if (block.receiver & ClientType::Client)
    {

        if (block.receiver >> 2 != socketClient->getId())
            qDebug() << QString("Receiver's id (%1) doesn't match my id (%2)").arg(block.receiver >> 2).arg(socketClient->getId());

        qDebug() << "New message from sender" << (block.sender >> 2); //сдвигаем на два бита, чтоб узнать id.

        execCommand(block);
    }
    else
    {
        qDebug() << "This block is not for client";
    }

}

void SocketClientChat::execCommand(const DataBlock &cmdBlock)
{
    switch (cmdBlock.command) {
    case DisplayMessage:
        displayMessage(QString::fromUtf8(cmdBlock.argument));
        break;
    default:
        break;
    }
}

void SocketClientChat::displayMessage(const QString &msg)
{
    //chat.append(msg);
    qDebug() << msg;
}

void SocketClientChat::sendInfo() const
{
    DataBlock block(ProvideInfo, socketClient->getName().toUtf8(),
                                     ClientType::Client, socketClient->getId());

    socketClient->sendBlock(block);
    qDebug() << "Info was sent to the server";
}

void SocketClientChat::sendMessage(const QString &msg)
{
    DataBlock block(DisplayMessage, msg.toUtf8(),
                    ClientType::Client, socketClient->getId(),
                    ClientType::All);

    socketClient->sendBlock(block);
    qDebug() << "Message" << msg << "was sent to everyone";
}
