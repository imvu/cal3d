#include "tinybind.h"
#include <sstream>

template<class T>
char const*
ConvertToString(T const& t) {
    std::stringstream str;
    static std::string strOut;
    str << t;
    strOut = str.str();
    return strOut.c_str();
}

template<class T>
void
ConvertFromString(char const* strIn, T* dataOut) {
    std::stringstream str;
    str << strIn;
    str >> *dataOut;
}

template<>
void
ConvertFromString<char const*>(char const* strIn, const char * * dataOut) {
    static std::string strHolder;
    strHolder = strIn;
    *dataOut = strHolder.c_str();
}

template<class T>
TiXmlBinding<T> const*
GetTiXmlBinding(T const&, IdentityBase) {
    static GenericTiXmlBinding<T> binding;
    return &binding;
}


TiXmlBinding<float> const*
GetTiXmlBinding(float const&, IdentityBase) {
    static GenericTiXmlBinding<float> binding;
    return &binding;
}

TiXmlBinding<double> const*
GetTiXmlBinding(double const&, IdentityBase) {
    static GenericTiXmlBinding<double> binding;
    return &binding;
}

TiXmlBinding<int> const*
GetTiXmlBinding(int const&, IdentityBase) {
    static GenericTiXmlBinding<int> binding;
    return &binding;
}

TiXmlBinding<char const*> const*
GetTiXmlBinding(char const* const&, IdentityBase) {
    static GenericTiXmlBinding<char const*> binding;
    return &binding;
}

TiXmlBinding<std::string> const*
GetTiXmlBinding(std::string const&, IdentityBase) {
    static GenericTiXmlBinding<std::string> binding;
    return &binding;
}

template void ConvertFromString<std::string>(char const* strIn, std::string* dataOut);
template void ConvertFromString<float>(char const* strIn, float* dataOut);
template void ConvertFromString<int>(char const* strIn, int* dataOut);
