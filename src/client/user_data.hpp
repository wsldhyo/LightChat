#ifndef USER_DATA_HPP
#define USER_DATA_HPP
#include <QString>
class SearchInfo {
public:
	SearchInfo(int uid, QString name, QString nick, QString desc, int sex);
	int _uid;
	QString _name;
	QString _nick;
	QString _desc;
	int _sex;
};
#endif