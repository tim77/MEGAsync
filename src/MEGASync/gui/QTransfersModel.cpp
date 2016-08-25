#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

bool priority_comparator(TransferItem* i, TransferItem *j)
{
    return (i->getPriority() < j->getPriority());
}

QTransfersModel::QTransfersModel(int type, QObject *parent) :
    QAbstractItemModel(parent)
{
    this->type = type;

    MegaApi *api =  ((MegaApplication *)qApp)->getMegaApi();
    delegateListener = new QTMegaTransferListener(api, this);
    api->addTransferListener(delegateListener);
}

TransferItem *QTransfersModel::transferFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<TransferItem *>(index.internalPointer());
    }
    else
    {
        return NULL;
    }
}

int QTransfersModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QTransfersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || transfers.count() <= index.row()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(static_cast<TransferItem *>(index.internalPointer()));
    }

    return QVariant();
}

QModelIndex QTransfersModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, transferOrder[row]);
}

void QTransfersModel::insertTransfer(MegaTransfer *transfer)
{
    TransferItem *item = new TransferItem();
    item->setPriority(transfer->getPriority());

    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);

    beginInsertRows(QModelIndex(), row, row);
    transfers.insert(transfer->getTag(), item);
    transferOrder.insert(it, item);
    endInsertRows();

    updateInitialTransferInfo(transfer);
    updateTransferInfo(transfer);

    if (transferOrder.size() == 1)
    {
        emit onTransferAdded();
    }
}

void QTransfersModel::removeTransfer(MegaTransfer *transfer)
{
    TransferItem *item =  transfers.value(transfer->getTag());
    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);

    beginRemoveRows(QModelIndex(), row, row);
    transfers.remove(transfer->getTag());
    transferOrder.erase(it);
    endRemoveRows();

    if (transfers.isEmpty())
    {
        emit noTransfers(type);
    }
}

int QTransfersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return transferOrder.size();
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
    delete delegateListener;
}

int QTransfersModel::getModelType()
{
    return type;
}

void QTransfersModel::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        insertTransfer(transfer);
    }
}

void QTransfersModel::onTransferFinish(MegaApi *, MegaTransfer *transfer, MegaError *)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        if (transfers.contains(transfer->getTag()))
        {
            TransferItem *item = transfers.value(transfer->getTag());
            item->finishTransfer();
            removeTransfer(transfer);
        }
    }
    else if (type == TYPE_FINISHED)
    {
        insertTransfer(transfer);
    }
}

void QTransfersModel::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        updateTransferInfo(transfer);
    }
}

void QTransfersModel::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError *e)
{

}

void QTransfersModel::setupModelTransfers(MegaTransferList *transfers)
{
    if (!transfers)
    {
        return;
    }

    for (int i = 0; i < transfers->size(); i++)
    {
        MegaTransfer *t = transfers->get(i);
        insertTransfer(t);
    }
    delete transfers;
}

void QTransfersModel::updateInitialTransferInfo(MegaTransfer *transfer)
{
    TransferItem *item = transfers.value(transfer->getTag());
    if (!item)
    {
        return;
    }

    item->setFileName(QString::fromUtf8(transfer->getFileName()));
    item->setType(transfer->getType(), transfer->isSyncTransfer());
    item->setTotalSize(transfer->getTotalBytes());

    //Update modified item
    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);
    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));

}
void QTransfersModel::updateTransferInfo(MegaTransfer *transfer)
{
    TransferItem *item = transfers.value(transfer->getTag());
    if (!item)
    {
        return;
    }

    item->setSpeed(transfer->getSpeed());
    item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
    type == TYPE_FINISHED ? item->finishTransfer() : item->updateTransfer();

    if (transfer->getPriority() == item->getPriority())
    {
        //Update modified item
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
    else
    {
        //Move item to its new position
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);

        item->setPriority(transfer->getPriority());
        auto newit = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int newrow = std::distance(transferOrder.begin(), newit);

        beginMoveRows(QModelIndex(), row, row, QModelIndex(), newrow);
        transferOrder.erase(it);
        auto finalit = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        transferOrder.insert(finalit, item);
        endMoveRows();
    }
}
