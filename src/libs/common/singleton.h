#pragma once

#include <stdio.h>
#include <stdlib.h>

namespace P2pClouds {

    template<typename T>
    class Singleton
    {
    public:
        static T* instance()
        {
            static T instance;
            return &instance;
        }

    private:
        Singleton();
        ~Singleton();
        Singleton(const Singleton &);
        Singleton & operator = (const Singleton &);
    };

}