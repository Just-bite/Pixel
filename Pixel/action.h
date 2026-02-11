#ifndef ACTION_H
#define ACTION_H

#include <stdio.h>

/*TODO command pattern lol*/

class Action
{

public:
    explicit Action();

private:
    size_t m_cache;
};

#endif // ACTION_H
