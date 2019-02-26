#include "ErrorCode.h"


const ErrorCode ErrorCode::EC_OK = { 0, "SUCCESS" };
const ErrorCode ErrorCode::EC_UNKNOWN_ERROR = { -1, "Unkown error" };
const ErrorCode ErrorCode::EC_BAD_PARAMETER = { -2, "Bad parameters" };
const ErrorCode ErrorCode::EC_BAD_TIME_SPAN = { -3, "Bad time span" };
const ErrorCode ErrorCode::EC_INVALID_USER_ID = { -4, "Invalid user ID" };
const ErrorCode ErrorCode::EC_INVALID_SERVER_INTERFACE = { -5, "No server interface" };
const ErrorCode ErrorCode::EC_GROUP_NOT_FOUND = { -6, "User group not found" };
const ErrorCode ErrorCode::EC_GET_PRICE_ERROR = { -7, "Fail to gurrent price" };
const ErrorCode ErrorCode::EC_SYMBOL_NOT_FOUND = { -8, "Symbol not found" };
const ErrorCode ErrorCode::EC_INVALID_ORDER_TICKET = { -9, "Invalid order ticket" };
const ErrorCode ErrorCode::EC_CHANGE_OPEN_PRICE = { -10, "Open price not allowed to modify for open order" };
const ErrorCode ErrorCode::EC_CLOSE_ONLY = { -11, "Update order is not allowed [close only]" };
const ErrorCode ErrorCode::EC_WRONG_PASSWORD = { -12, "Password at lest 6 characters" };
const ErrorCode ErrorCode::EC_PENDING_ORDER_WITHOUT_OPEN_PRICE = { -13, "Pending order without open price" };
const ErrorCode ErrorCode::EC_USER_ID_EXISTING = { -14, "User ID already used" };
const ErrorCode ErrorCode::EC_NO_MEMORY = { -15, "No memory to perform the request" };

const ErrorCode ErrorCode::EC_NO_CONNECT = { 6, "No connection" };
const ErrorCode ErrorCode::EC_ACCOUNT_DISABLED = { 64, "Account blocked" };
const ErrorCode ErrorCode::EC_BAD_ACCOUNT_INFO = { 65, "Bad account info" };
const ErrorCode ErrorCode::EC_TRADE_TIMEOUT = { 128, "Trade transatcion timeou expired" };
const ErrorCode ErrorCode::EC_TRADE_BAD_PRICES = { 129, "Order has wrong prices" };
const ErrorCode ErrorCode::EC_TRADE_BAD_STOPS = { 130, "Wrong stops level" };
const ErrorCode ErrorCode::EC_TRADE_BAD_VOLUME = { 131, "Wrong lot size" };
const ErrorCode ErrorCode::EC_TRADE_MARKET_CLOSED = { 132, "Market closed" };
const ErrorCode ErrorCode::EC_TRADE_DISABLE = { 133, "Trade disabled" };
const ErrorCode ErrorCode::EC_TRADE_NO_MONEY = { 134, "No enough money for order execution" };
const ErrorCode ErrorCode::EC_TRADE_PRICE_CHANGED = { 135, "Price changed" };
const ErrorCode ErrorCode::EC_TRADE_OFFQUOTES = { 136, "No quotes" };
const ErrorCode ErrorCode::EC_TRADE_BROKER_BUSY = { 137, "Broker is busy" };
const ErrorCode ErrorCode::EC_TRADE_ORDER_LOCKED = { 138, "Order is proceed by dealer and cannot be changed" };
const ErrorCode ErrorCode::EC_TRADE_LONG_ONLY = { 139, "Allowed only BUY orders" };
const ErrorCode ErrorCode::EC_TRADE_TOO_MANY_REQ = { 140, "Too many requests from one client" };
const ErrorCode ErrorCode::EC_TRADE_MODIFY_DENIED = { 144, "Modification denied because order too close to market" };

const ErrorCode ErrorCode::EC_MAN_ERROR = { 256, "Manager API error" };
const ErrorCode ErrorCode::EC_MAN_NO_CONNECT = { 257, "Manager API No connection" };
const ErrorCode ErrorCode::EC_MAN_NO_AVAILABLE_INTERFACE = { 257, "Manager API No available interface to perform the request" };
