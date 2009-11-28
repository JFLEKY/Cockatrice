#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QVariant>
#include "protocol_item_ids.h"
#include "protocol_datastructures.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;

class ProtocolResponse;
class DeckList;

enum ItemId {
	ItemId_Command_DeckUpload = ItemId_Other + 100,
	ItemId_Command_DeckSelect = ItemId_Other + 101,
	ItemId_Event_ListChatChannels = ItemId_Other + 200,
	ItemId_Event_ChatListPlayers = ItemId_Other + 201,
	ItemId_Event_ListGames = ItemId_Other + 202,
	ItemId_Event_GameStateChanged = ItemId_Other + 203,
	ItemId_Event_CreateArrow = ItemId_Other + 204,
	ItemId_Event_CreateCounter = ItemId_Other + 205,
	ItemId_Event_DrawCards = ItemId_Other + 206,
	ItemId_Response_DeckList = ItemId_Other + 300,
	ItemId_Response_DeckDownload = ItemId_Other + 301,
	ItemId_Response_DeckUpload = ItemId_Other + 302
};

class ProtocolItem : public QObject {
	Q_OBJECT
private:
	QString currentElementText;
protected:
	typedef ProtocolItem *(*NewItemFunction)();
	static QHash<QString, NewItemFunction> itemNameHash;
	
	QString itemName;
	QMap<QString, QString> parameters;
	void setParameter(const QString &name, const QString &value) { parameters[name] = value; }
	void setParameter(const QString &name, bool value) { parameters[name] = (value ? "1" : "0"); }
	void setParameter(const QString &name, int value) { parameters[name] = QString::number(value); }
	void setParameter(const QString &name, const QColor &value) { parameters[name] = QString::number(ColorConverter::colorToInt(value)); }
	virtual void extractParameters() { }
	virtual QString getItemType() const = 0;
	
	virtual bool readElement(QXmlStreamReader * /*xml*/) { return false; }
	virtual void writeElement(QXmlStreamWriter * /*xml*/) { }
private:
	static void initializeHashAuto();
public:
	static const int protocolVersion = 4;
	virtual int getItemId() const = 0;
	ProtocolItem(const QString &_itemName);
	static void initializeHash();
	static ProtocolItem *getNewItem(const QString &name);
	bool read(QXmlStreamReader *xml);
	void write(QXmlStreamWriter *xml);
};

// ----------------
// --- COMMANDS ---
// ----------------

class Command : public ProtocolItem {
	Q_OBJECT
signals:
	void finished(ProtocolResponse *response);
	void finished(ResponseCode response);
private:
	int cmdId;
	int ticks;
	static int lastCmdId;
	QVariant extraData;
protected:
	QString getItemType() const { return "cmd"; }
	void extractParameters();
public:
	Command(const QString &_itemName = QString(), int _cmdId = -1);
	int getCmdId() const { return cmdId; }
	int tick() { return ++ticks; }
	void processResponse(ProtocolResponse *response);
	void setExtraData(const QVariant &_extraData) { extraData = _extraData; }
	QVariant getExtraData() const { return extraData; }
};

class InvalidCommand : public Command {
	Q_OBJECT
public:
	InvalidCommand() : Command() { }
	int getItemId() const { return ItemId_Other; }
};

class ChatCommand : public Command {
	Q_OBJECT
private:
	QString channel;
protected:
	void extractParameters()
	{
		Command::extractParameters();
		channel = parameters["channel"];
	}
public:
	ChatCommand(const QString &_cmdName, const QString &_channel)
		: Command(_cmdName), channel(_channel)
	{
		setParameter("channel", channel);
	}
	QString getChannel() const { return channel; }
};

class GameCommand : public Command {
	Q_OBJECT
private:
	int gameId;
protected:
	void extractParameters()
	{
		Command::extractParameters();
		gameId = parameters["game_id"].toInt();
	}
public:
	GameCommand(const QString &_cmdName, int _gameId)
		: Command(_cmdName), gameId(_gameId)
	{
		setParameter("game_id", gameId);
	}
	int getGameId() const { return gameId; }
	void setGameId(int _gameId)
	{
		gameId = _gameId;
		setParameter("game_id", gameId);
	}
};

class Command_DeckUpload : public Command {
	Q_OBJECT
private:
	DeckList *deck;
	QString path;
	bool readFinished;
protected:
	void extractParameters();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Command_DeckUpload(DeckList *_deck = 0, const QString &_path = QString());
	static ProtocolItem *newItem() { return new Command_DeckUpload; }
	int getItemId() const { return ItemId_Command_DeckUpload; }
	DeckList *getDeck() const { return deck; }
	QString getPath() const { return path; }
};

class Command_DeckSelect : public GameCommand {
	Q_OBJECT
private:
	DeckList *deck;
	int deckId;
	bool readFinished;
protected:
	void extractParameters();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Command_DeckSelect(int _gameId = -1, DeckList *_deck = 0, int _deckId = -1);
	static ProtocolItem *newItem() { return new Command_DeckSelect; }
	int getItemId() const { return ItemId_Command_DeckSelect; }
	DeckList *getDeck() const { return deck; }
	int getDeckId() const { return deckId; }
};

// -----------------
// --- RESPONSES ---
// -----------------

class ProtocolResponse : public ProtocolItem {
	Q_OBJECT
private:
	int cmdId;
	ResponseCode responseCode;
	static QHash<QString, ResponseCode> responseHash;
protected:
	QString getItemType() const { return "resp"; }
	void extractParameters();
public:
	ProtocolResponse(int _cmdId = -1, ResponseCode _responseCode = RespOk, const QString &_itemName = QString());
	int getItemId() const { return ItemId_Other; }
	static void initializeHash();
	static ProtocolItem *newItem() { return new ProtocolResponse; }
	int getCmdId() const { return cmdId; }
	ResponseCode getResponseCode() const { return responseCode; }
};

class Response_DeckList : public ProtocolResponse {
	Q_OBJECT
private:
	DeckList_Directory *root;
	bool readFinished;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Response_DeckList(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList_Directory *_root = 0);
	~Response_DeckList();
	int getItemId() const { return ItemId_Response_DeckList; }
	static ProtocolItem *newItem() { return new Response_DeckList; }
	DeckList_Directory *getRoot() const { return root; }
};

class Response_DeckDownload : public ProtocolResponse {
	Q_OBJECT
private:
	DeckList *deck;
	bool readFinished;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Response_DeckDownload(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList *_deck = 0);
	int getItemId() const { return ItemId_Response_DeckDownload; }
	static ProtocolItem *newItem() { return new Response_DeckDownload; }
	DeckList *getDeck() const { return deck; }
};

class Response_DeckUpload : public ProtocolResponse {
	Q_OBJECT
private:
	DeckList_File *file;
	bool readFinished;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Response_DeckUpload(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList_File *_file = 0);
	~Response_DeckUpload();
	int getItemId() const { return ItemId_Response_DeckUpload; }
	static ProtocolItem *newItem() { return new Response_DeckUpload; }
	DeckList_File *getFile() const { return file; }
};

// --------------
// --- EVENTS ---
// --------------

class GenericEvent : public ProtocolItem {
	Q_OBJECT
protected:
	QString getItemType() const { return "generic_event"; }
public:
	GenericEvent(const QString &_eventName);
};

class GameEvent : public ProtocolItem {
	Q_OBJECT
private:
	int gameId;
	int playerId;
protected:
	QString getItemType() const { return "game_event"; }
	void extractParameters();
public:
	GameEvent(const QString &_eventName, int _gameId, int _playerId);
	int getGameId() const { return gameId; }
	int getPlayerId() const { return playerId; }
	void setGameId(int _gameId)
	{
		gameId = _gameId;
		setParameter("game_id", gameId);
	}
};

class ChatEvent : public ProtocolItem {
	Q_OBJECT
private:
	QString channel;
protected:
	QString getItemType() const { return "chat_event"; }
	void extractParameters();
public:
	ChatEvent(const QString &_eventName, const QString &_channel);
	QString getChannel() const { return channel; }
};

class Event_ListChatChannels : public GenericEvent {
	Q_OBJECT
private:
	QList<ServerChatChannelInfo> channelList;
public:
	Event_ListChatChannels() : GenericEvent("list_chat_channels") { }
	int getItemId() const { return ItemId_Event_ListChatChannels; }
	static ProtocolItem *newItem() { return new Event_ListChatChannels; }
	void addChannel(const QString &_name, const QString &_description, int _playerCount, bool _autoJoin)
	{
		channelList.append(ServerChatChannelInfo(_name, _description, _playerCount, _autoJoin));
	}
	const QList<ServerChatChannelInfo> &getChannelList() const { return channelList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_ChatListPlayers : public ChatEvent {
	Q_OBJECT
private:
	QList<ServerChatUserInfo> playerList;
public:
	Event_ChatListPlayers(const QString &_channel = QString()) : ChatEvent("chat_list_players", _channel) { }
	int getItemId() const { return ItemId_Event_ChatListPlayers; }
	static ProtocolItem *newItem() { return new Event_ChatListPlayers; }
	void addPlayer(const QString &_name)
	{
		playerList.append(ServerChatUserInfo(_name));
	}
	const QList<ServerChatUserInfo> &getPlayerList() const { return playerList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_ListGames : public GenericEvent {
	Q_OBJECT
private:
	QList<ServerGameInfo> gameList;
public:
	Event_ListGames() : GenericEvent("list_games") { }
	int getItemId() const { return ItemId_Event_ListGames; }
	static ProtocolItem *newItem() { return new Event_ListGames; }
	void addGame(int _gameId, const QString &_description, bool _hasPassword, int _playerCount, int _maxPlayers, const QString &_creatorName, bool _spectatorsAllowed, int _spectatorCount)
	{
		gameList.append(ServerGameInfo(_gameId, _description, _hasPassword, _playerCount, _maxPlayers, _creatorName, _spectatorsAllowed, _spectatorCount));
	}
	const QList<ServerGameInfo> &getGameList() const { return gameList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_GameStateChanged : public GameEvent {
	Q_OBJECT
private:
	SerializableItem *currentItem;
	QList<ServerInfo_Player *> playerList;
public:
	Event_GameStateChanged(int _gameId = -1, const QList<ServerInfo_Player *> &_playerList = QList<ServerInfo_Player *>());
	~Event_GameStateChanged();
	const QList<ServerInfo_Player *> &getPlayerList() const { return playerList; }
	static ProtocolItem *newItem() { return new Event_GameStateChanged; }
	int getItemId() const { return ItemId_Event_GameStateChanged; }
	
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_CreateArrow : public GameEvent {
	Q_OBJECT
private:
	ServerInfo_Arrow *arrow;
	bool readFinished;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Event_CreateArrow(int _gameId = -1, int _playerId = -1, ServerInfo_Arrow *_arrow = 0);
	~Event_CreateArrow();
	int getItemId() const { return ItemId_Event_CreateArrow; }
	static ProtocolItem *newItem() { return new Event_CreateArrow; }
	ServerInfo_Arrow *getArrow() const { return arrow; }
};

class Event_CreateCounter : public GameEvent {
	Q_OBJECT
private:
	ServerInfo_Counter *counter;
	bool readFinished;
protected:
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Event_CreateCounter(int _gameId = -1, int _playerId = -1, ServerInfo_Counter *_counter = 0);
	~Event_CreateCounter();
	int getItemId() const { return ItemId_Event_CreateCounter; }
	static ProtocolItem *newItem() { return new Event_CreateCounter; }
	ServerInfo_Counter *getCounter() const { return counter; }
};

class Event_DrawCards : public GameEvent {
	Q_OBJECT
private:
	SerializableItem *currentItem;
	int numberCards;
	QList<ServerInfo_Card *> cardList;
protected:
	void extractParameters();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
public:
	Event_DrawCards(int _gameId = -1, int _playerId = -1, int numberCards = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>());
	~Event_DrawCards();
	int getItemId() const { return ItemId_Event_DrawCards; }
	static ProtocolItem *newItem() { return new Event_DrawCards; }
	int getNumberCards() const { return numberCards; }
	const QList<ServerInfo_Card *> &getCardList() const { return cardList; }
};

#endif