/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "playlistmanager.h"

#include <QFileInfo>
#include <QDebug>
#include <QUuid>

#include "../musicapp.h"

static const QString DefaultListID = "Default";
const QString AllMusicListID = "All";
const QString FavMusicListID = "Fav";
static MusicListInfo emptyInfo;

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent), settings(MusicApp::configPath() + "/Playlist.ini", QSettings::IniFormat)
{
}

PlaylistManager::~PlaylistManager()
{
    settings.sync();
}

QString PlaylistManager::newID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString PlaylistManager::newDisplayName()
{
    QMap<QString, QString> existName;

    settings.beginGroup("PlaylistManager");
    auto keys = settings.value("SortPlaylist").toStringList();
    settings.endGroup();

    settings.beginGroup("PlaylistEntry");
    for (auto &key : keys) {
        auto name = settings.value(key).toString();
        existName.insert(name, name);
    }
    settings.endGroup();

    QString temp = tr("New Playlist");
    if (!existName.contains(temp)) {
        return temp;
    }

    int i = 1;
    for (i = 1; i < existName.size() + 1; ++i) {
        QString newName = QString("%1 %2").arg(temp).arg(i);
        if (!existName.contains(newName)) {
            return newName;
        }
    }
    return QString("%1 %2").arg(temp).arg(i);
}

void PlaylistManager::load()
{
    settings.beginGroup("PlaylistManager");
    auto currentTitle = settings.value("Current").toString();
    sortPlaylists = settings.value("SortPlaylist").toStringList();
    settings.endGroup();

    for (auto &playlistid : sortPlaylists) {
        auto playlistPath = getPlaylistPath(playlistid);
        QFileInfo fi(playlistPath);
        if (!fi.exists()) {
            qWarning() << "playlist file not exist" << playlistid
                       << fi.absoluteFilePath();
            continue;
        }
        MusicListInfo info;
        info.url = playlistPath;
        auto emptylist = QSharedPointer<Playlist>(new Playlist(info));
        emptylist->load();
        insertPlaylist(playlistid, emptylist);
    }

    if (playlist(AllMusicListID).isNull()) {
        MusicListInfo info;
        info.id = AllMusicListID;
        info.editmode = false;
        info.icon = "all";
        info.readonly = true;
        info.displayName = tr("All Music");
        addPlaylist(info);
        playlist(AllMusicListID)->save();
    }

    if (playlist(FavMusicListID).isNull()) {
        MusicListInfo info;
        info.id = FavMusicListID;
        info.editmode = false;
        info.icon = "fav";
        info.readonly = true;
        info.displayName = tr("Favator Music");
        addPlaylist(info);
        playlist(FavMusicListID)->save();
    }
    m_currentPlaylist = playlist(currentTitle);
    if (m_currentPlaylist.isNull()) {
        qDebug() << "change to default all palylist";
        m_currentPlaylist = playlist(AllMusicListID);
    }
}

void PlaylistManager::sync()
{
    settings.sync();
}

QList<QSharedPointer<Playlist> > PlaylistManager::allplaylist()
{
    QList<QSharedPointer<Playlist> >  list;
    for (auto &playlist : sortPlaylists) {
        list << playlists.value(playlist);
    }
    return list;
}

QSharedPointer<Playlist> PlaylistManager::addPlaylist(const MusicListInfo &listinfo)
{
    MusicListInfo saveInfo(listinfo);
    QString playlistPath = getPlaylistPath(listinfo.id);
    saveInfo.url = playlistPath;
    insertPlaylist(listinfo.id, QSharedPointer<Playlist>(new Playlist(saveInfo)));
    sortPlaylists << listinfo.id;

    settings.beginGroup("PlaylistManager");
    settings.setValue("SortPlaylist", sortPlaylists);
    settings.endGroup();

    settings.beginGroup("PlaylistEntry");
    settings.setValue(listinfo.id, listinfo.displayName);
    settings.endGroup();

    settings.sync();

    playlists.value(listinfo.id)->save();
    return playlists.value(listinfo.id);
}

QSharedPointer<Playlist> PlaylistManager::playlist(const QString &id)
{
    return playlists.value(id);
}

QSharedPointer<Playlist> PlaylistManager::currentPlaylist() const
{
    return m_currentPlaylist;
}

void PlaylistManager::setCurrentPlaylist(QSharedPointer<Playlist> currentPlaylist)
{
    if (m_currentPlaylist == currentPlaylist) {
        return;
    }
    m_currentPlaylist = currentPlaylist;
    emit currentPlaylistChanged(currentPlaylist);

    settings.beginGroup("PlaylistManager");
    settings.setValue("Current", m_currentPlaylist->id());
    settings.endGroup();
    settings.sync();
}

QString PlaylistManager::getPlaylistPath(const QString &id)
{
    return MusicApp::configPath() + "/" + id + ".plsx";
}

void PlaylistManager::insertPlaylist(const QString &id, QSharedPointer<Playlist> playlist)
{
    QString deleteID = id;

    connect(playlist.data(), &Playlist::removed,
    this, [ = ] {
        if (m_currentPlaylist.isNull() || m_currentPlaylist->id() == deleteID)
        {
            setCurrentPlaylist(this->playlist(AllMusicListID));
        }
        QFile::remove(getPlaylistPath(deleteID));
        playlists.remove(deleteID);
        sortPlaylists.removeAll(deleteID);

        settings.beginGroup("PlaylistManager");
        settings.setValue("SortPlaylist", sortPlaylists);
        settings.endGroup();

        if (!deleteID.isEmpty())
        {
            settings.beginGroup("PlaylistEntry");
            settings.remove(deleteID);
            settings.endGroup();
        }

        settings.sync();
    });

    playlists.insert(id, playlist);
}
