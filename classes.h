#include "include/configkey.h"

using namespace QJsonUtil;

struct Student : JsonDumpInterface {

    CONFIG_KEY(QString, name);

    CONFIG_KEY(int, age);

    //字段与key名称不一致时不使用宏
    ConfigKey<double> scoreAvg{"score_avg"};

    CONFIG_KEY(QStringList, adept);

    QList<JsonReadInterface *> prop() override {
        return {&name, &age, &scoreAvg, &adept};
    }
};

struct Teacher : JsonDumpInterface {

    CONFIG_KEY(QString, name);

    CONFIG_KEY(double, score);

    QList<JsonReadInterface *> prop() override {
        return {&name, &score};
    }
};

struct Classes : JsonDumpInterface {

    CONFIG_KEY(QString, name);

    CONFIG_KEY(int, room);

    CONFIG_KEY(QStringList, courses);

    CONFIG_KEY(Teacher, teacher);

    CONFIG_KEY(QList<Student>, students);

    CONFIG_KEY(QList<int>, types);

    QList<JsonReadInterface *> prop() override {
        return {&name, &room, &courses, &teacher, &students, &types };
    }
};