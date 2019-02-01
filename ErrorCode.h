#ifndef _ERRORCODE_H_
#define _ERRORCODE_H_
struct ErrorCode {
    // predefined const error codes
    static const ErrorCode EC_OK;
    static const ErrorCode EC_UNKNOWN_ERROR;
    static const ErrorCode EC_BAD_PARAMETER;
    static const ErrorCode EC_BAD_TIME_SPAN;
    static const ErrorCode EC_INVALID_USER_ID;
    static const ErrorCode EC_INVALID_SERVER_INTERFACE;
    static const ErrorCode EC_GROUP_NOT_FOUND;
    static const ErrorCode EC_GET_PRICE_ERROR;
    static const ErrorCode EC_SYMBOL_NOT_FOUND;
    static const ErrorCode EC_INVALID_ORDER_TICKET;
    static const ErrorCode EC_CHANGE_OPEN_PRICE;
    static const ErrorCode EC_CLOSE_ONLY;
    static const ErrorCode EC_WRONG_PASSWORD;
    static const ErrorCode EC_PENDING_ORDER_WITHOUT_OPEN_PRICE;
    static const ErrorCode EC_USER_ID_EXISTING;

    static const ErrorCode EC_NO_CONNECT;
    static const ErrorCode EC_ACCOUNT_DISABLED;
    static const ErrorCode EC_BAD_ACCOUNT_INFO;
    static const ErrorCode EC_TRADE_TIMEOUT;
    static const ErrorCode EC_TRADE_BAD_PRICES;
    static const ErrorCode EC_TRADE_BAD_STOPS;
    static const ErrorCode EC_TRADE_BAD_VOLUME;
    static const ErrorCode EC_TRADE_MARKET_CLOSED;
    static const ErrorCode EC_TRADE_DISABLE;
    static const ErrorCode EC_TRADE_NO_MONEY;
    static const ErrorCode EC_TRADE_PRICE_CHANGED;
    static const ErrorCode EC_TRADE_OFFQUOTES;
    static const ErrorCode EC_TRADE_BROKER_BUSY;
    static const ErrorCode EC_TRADE_ORDER_LOCKED;
    static const ErrorCode EC_TRADE_LONG_ONLY;
    static const ErrorCode EC_TRADE_TOO_MANY_REQ;
    static const ErrorCode EC_TRADE_MODIFY_DENIED;

    static const ErrorCode EC_MAN_ERROR;
    static const ErrorCode EC_MAN_NO_CONNECT;
    static const ErrorCode EC_MAN_NO_AVAILABLE_INTERFACE;
    int m_code;
    const char* m_des;
};

#endif  // !_ERRORCODE_H_
