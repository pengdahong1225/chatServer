#ifndef _SINGLETON_H_
#define _SINGLETON_H_

/* CSingleton */
template<typename ObjectType>
class CSinleton{
public:
    static ObjectType* Instance()
    {
        return &Reference();
    }
    static ObjectType& Reference()
    {
        static ObjectType _Instance;
        return _Instance;
    }
protected:
    CSinleton(){};
    CSinleton(CSinleton const&){}
};

#endif