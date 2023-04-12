#include "AsycNetStatus.h"
#include "RewriteApi/GoStr.h"
#include "RewriteApi/CFCFileApiParse.h"

AsycNetStatus::AsycNetStatus(int fc, QObject *parent)
    : QObject{parent}
{
    m_fc = fc;
    m_Stop = false;

    m_ErrNum = 0;

    reConnNum = 3;

    isReCon = true;


    m_CountDown = nullptr;
}

void AsycNetStatus::stop()
{
    m_Stop = true;
}

void AsycNetStatus::setfc(int fc)
{
    m_fc = fc;
}

void AsycNetStatus::slotNoReConn()
{
    isReCon = false;
}

void AsycNetStatus::setCountDown(CountDown *p)
{
    m_CountDown = p;
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
        bool exitC = true;
        num = 0;

        if(netStatus.delayErr.isEmpty()) {
            emit sigDelay(netStatus.delay);
        } else if(netStatus.delayErr.contains("is inactive") || netStatus.delayErr.contains("is disable")) {
            if(netStatus.delayErr.contains("is disable")) {
                  exitC = false;
            }
            ++num;
        }

        if(netStatus.rootSizeErr.isEmpty()) {
            emit sigMemoryCapacity(netStatus.rootSize, 0, netStatus.maxRootSize, netStatus.rootSizeShow, netStatus.maxRootSizeShow);
        }  else if(netStatus.rootSizeErr.contains("sub box is disable")){
            ++num;
        }

        if(num == 0) {
            m_ErrNum = 0;
            reConnNum = 3;
            isReCon = true;

//            if(m_CountDown) {
//                m_CountDown->hide();
//            }
        } else {
            int num1 = reConnNum - m_ErrNum;
            ++m_ErrNum;
            if(!isReCon) {
                num1 = 0;
            }
            if(m_ErrNum >= reConnNum) {
                m_ErrNum = 0;
                if(reConnNum == 3) {
                    reConnNum = 0;
                }
                reConnNum += ReConnStep;
                if(reConnNum >= ReConnMax+ReConnStep) {
                    reConnNum = ReConnMax;
                    isReCon = false;
                }
                emit sigReConnection(num1, exitC, isReCon);
            } else {
                emit sigReConnection(num1, exitC, false);
            }

        }

        emit sigNetworkSpeed(netStatus.uploadSpeed, netStatus.uploadSpeedShow, netStatus.downloadSpeed, netStatus.downloadSpeedShow, num == 0);

        QThread::msleep(1000);
    }
}
