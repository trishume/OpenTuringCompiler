#ifndef _IDMANAGER_H_
#define _IDMANAGER_H_

#include <map>
#include <string>
#include <sstream>

#include "openTuringLibDefs.h"
#include "openTuringRuntimeError.h"

template <class T> class IDManager {
public:
    IDManager(std::string typeName) : LastId(-1), TypeName(typeName){}
    ~IDManager();
    T *get(TInt id);
    bool exists(TInt id);
    
    // life cycle
    TInt getNew();
    void remove(TInt id);
protected:
    void assertExists(TInt id);
    
    std::map<TInt,T *> Items;
    TInt LastId;
    std::string TypeName;
};

#pragma mark Methods

template <class T>
IDManager<T>::~IDManager() {
    for (unsigned int i = 0; i < Items.size(); ++i) {
        T *curItem = Items[i];
        delete curItem;
    }
}

template <class T>
T *IDManager<T>::get(TInt id) {
    assertExists(id);
    return Items[id];
}

template <class T>
bool  IDManager<T>::exists(TInt id) {
    return Items.find(id) != Items.end();
}

template <class T>
TInt IDManager<T>::getNew() {
    T *newItem = new T();
    LastId += 1;
    Items[LastId] = newItem;
    return LastId;
}

template <class T> 
void IDManager<T>::remove(TInt id) {
    T *win = get(id);
    Items.erase(id);
    delete win;
}

#pragma mark Protected and Private Methods

template <class T>
void IDManager<T>::assertExists(TInt id) {
    if (!exists(id)) {
        std::ostringstream os;
    	os << TypeName << " ID " << id << " does not exist.";
        turingRuntimeError(os.str().c_str());
    }
}
#endif