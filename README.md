# qjsonutil
- configkey.h  
c++结构体与json字符串相互转换工具类

json字符串：
```json
{
    "name": "class1",
    "room": 1,
    "courses": ["math", "english", "physics", "chemistry", "biology"],
    "teacher": {
        "name": "Tony",
        "score": 99.9
    },
    "students": [
        {
            "name": "Alice",
            "age": 12,
            "score_avg": 90.4,
            "adept": ["math", "english"]
        },
        {
            "name": "Bob",
            "age": 13,
            "score_avg": 86.1,
            "adept": ["physics", "chemistry"]
        }
    ]
}
```

定义模型：

```cpp
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

    QList<JsonReadInterface *> prop() override {
        return {&name, &room, &courses, &teacher, &students};
    }
};
```

转换：

```cpp
auto doc = QJsonDocument::fromJson(jsonStr);
if (!doc.isNull()) {
  auto object = doc.object();

  Classes classes;
  classes.fromJson(object);

  auto json = classes.dumpToJson();
}
```
