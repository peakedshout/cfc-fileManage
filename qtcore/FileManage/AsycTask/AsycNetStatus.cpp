#include "AsycNetStatus.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/CFCFileApiParse.h"

AsycNetStatus::AsycNetStatus(int fc, QObject *parent)
    : QObject{parent}
{
    m_fc = fc;
    m_Stop = false;
}

void AsycNetStatus::stop()
{
    m_Stop = true;
}

void AsycNetStatus::run()
{
    m_Stop = false;
    while (true) {
        if(m_Stop) return;
        QSharedPointer<char> msg(GetRemoteStatusFileContext(m_fc));

        debugMsg(msg.data());

        NetStatus netStatus;
        CFCFileApiParse::netStatusContext(msg.data(), netStatus);

        if(netStatus.delayErr.isEmpty()) {
            emit sigDelay(netStatus.delay);
        }

        if(netStatus.rootSizeErr.isEmpty()) {
            emit sigMemoryCapacity(netStatus.rootSize, 0, netStatus.maxRootSize, netStatus.rootSizeShow, netStatus.maxRootSizeShow);
        }

        emit sigNetworkSpeed(netStatus.uploadSpeed, netStatus.uploadSpeedShow, netStatus.downloadSpeed, netStatus.downloadSpeedShow);

        QThread::msleep(1500);
    }
}
