/*
  www.sourceforge.net/projects/tinyxml
  Original code copyright (c) 2004 Eric Ries (tinybind-eric@sneakemail.com)

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must
  not claim that you wrote the original software. If you use this
  software in a product, an acknowledgment in the product documentation
  would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
  must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source
  distribution.
*/


#pragma once

#include "tinyxml.h"
#include "platform.h"

#include <vector>
#include <list>

struct IdentityBase {
};

template<class T>
struct Identity : public IdentityBase {
    typedef T type;
};

template<class T>
class TiXmlBinding {
public:
    virtual bool fromXml(TiXmlElement const& elem, T* data) const = 0;
};

template<class T>
bool
BindFromXml(TiXmlElement const& elemIn, T* dataOut) {
    TiXmlBinding<T> const* binding = GetTiXmlBinding(*dataOut, Identity<T>());
    return binding->fromXml(elemIn, dataOut);
}


template<class T>
class IMemberHolder {
public:
    const char* tag_;

    virtual char const* tag() const {
        return tag_;
    }

    virtual bool fromXml(TiXmlElement const&, T*) = 0;
    virtual bool isAttributeMember() = 0;
};



template<class T, class MT>
class IMemberValuePolicy {
public:
    virtual MT const& getMemberValue(T const* thisPtr) = 0;
    virtual void setMemberValue(T* thisPtr, MT const& mv) = 0;
};


template<class T, class MT>
class MemberPtrHolder : public IMemberValuePolicy<T, MT> {
public:
    MT T::* memberPtr_;
    virtual MT const& getMemberValue(T const* thisPtr) {
        return thisPtr->*memberPtr_;
    }
    virtual void setMemberValue(T* thisPtr, MT const& mv) {
        thisPtr->*memberPtr_ = mv;
    }
};

template<class T, class MT>
class FromXmlElement : public IMemberHolder<T> {
public:
    IMemberValuePolicy<T, MT> * mvPolicy_;
    FromXmlElement(IMemberValuePolicy<T, MT> * mvPolicy) {
        mvPolicy_ = mvPolicy;
    }

    virtual bool fromXml(TiXmlElement const& elem, T* thisPtr) {
        MT& mv = const_cast<MT&>(mvPolicy_->getMemberValue(thisPtr));
        TiXmlBinding<MT> const* binding = GetTiXmlBinding(mv,  Identity<MT>());
        if (binding->fromXml(elem, &mv)) {
            mvPolicy_->setMemberValue(thisPtr, mv);
            return true;
        } else {
            return false;
        }
    }

    virtual bool isAttributeMember() {
        return true;
    }
};


template<class T, class MT, class FromXmlPolicy, class MemberValuePolicy>
class MemberHolder {
public:
    FromXmlPolicy xmlPolicy_;
    MemberValuePolicy mvPolicy_;

    MemberHolder()
        : xmlPolicy_((IMemberValuePolicy<T, MT> *)&mvPolicy_) {
    }
};



template<class T, class MT>
class FromXmlChildElement : public IMemberHolder<T> {
public:
    IMemberValuePolicy<T, MT> * mvPolicy_;
    FromXmlChildElement(IMemberValuePolicy<T, MT> * mvPolicy) {
        mvPolicy_ = mvPolicy;
    }

    virtual bool fromXml(TiXmlElement const& elem, T* thisPtr) {
        if (!cal3d_stricmp(elem.Value(), IMemberHolder<T>::tag())) {
            MT mv;
            TiXmlBinding<MT> const* binding = GetTiXmlBinding(mv,  Identity<MT>());
            if (binding->fromXml(elem, &mv)) {
                mvPolicy_->setMemberValue(thisPtr, mv);
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    virtual bool isAttributeMember() {
        return false;
    }
};

template<class T>
void ConvertFromString(char const* strIn, T* dataOut);

template<class T, class MT>
class FromXmlAttribute  : public IMemberHolder<T> {
public:
    IMemberValuePolicy<T, MT> * mvPolicy_;
    FromXmlAttribute(IMemberValuePolicy<T, MT> * mvPolicy) {
        mvPolicy_ = mvPolicy;
    }

    virtual bool fromXml(TiXmlElement const& elem, T* thisPtr) {
        MT mv;
        const char* attributeValue = elem.Attribute(IMemberHolder<T>::tag());
        if (attributeValue && *attributeValue) {
            ConvertFromString(attributeValue, &mv);
            mvPolicy_->setMemberValue(thisPtr, mv);
            return true;
        } else {
            return false;
        }
    }

    virtual bool isAttributeMember() {
        return true;
    }
};

template<class T, class MT>
IMemberHolder<T> * Member(MT T::*mp) {
    typedef FromXmlChildElement<T, MT> XmlPolicy;
    typedef MemberPtrHolder<T, MT> MemberValuePolicy;
    typedef MemberHolder<T, MT, XmlPolicy, MemberValuePolicy> MH_Type;
    MH_Type* mph = new MH_Type();
    mph->mvPolicy_.memberPtr_ = mp;
    return &mph->xmlPolicy_;
}

template<class T, class MT>
IMemberHolder<T>* MemberAttribute(MT T::*mp) {
    typedef FromXmlAttribute<T, MT> XmlPolicy;
    typedef MemberPtrHolder<T, MT> MemberValuePolicy;
    typedef MemberHolder<T, MT, XmlPolicy, MemberValuePolicy> MH_Type;
    MH_Type* mph = new MH_Type();
    mph->mvPolicy_.memberPtr_ = mp;
    return &mph->xmlPolicy_;
}

template<class T, class MT>
IMemberHolder<T> * MemberPeer(MT T::*mp) {
    typedef FromXmlElement<T, MT> XmlPolicy;
    typedef MemberPtrHolder<T, MT> MemberValuePolicy;
    typedef MemberHolder<T, MT, XmlPolicy, MemberValuePolicy> MH_Type;
    MH_Type* mph = new MH_Type();
    mph->mvPolicy_.memberPtr_ = mp;
    return &mph->xmlPolicy_;
}

template<class T>
class MemberTiXmlBinding : public TiXmlBinding<T> {
private:
    std::vector<IMemberHolder<T> *> members_;

public:
    bool empty() const {
        return members_.empty();
    }

    IMemberHolder<T> * AddMember(const char* tag, IMemberHolder<T> * mph) {
        mph->tag_ = tag;
        members_.push_back(mph);
        return mph;
    }

    virtual bool fromXml(TiXmlElement const& elem, T* data) const {
        TiXmlElement const* child = elem.FirstChildElement();
        for (size_t i = 0; i < members_.size(); i++) {
            IMemberHolder<T> * mph = members_[i];
            bool error = false;

            bool ret;
            if (mph->isAttributeMember()) {
                ret = mph->fromXml(elem, data);
            } else {
                if (!child) {
                    return false;
                }
                ret = mph->fromXml(*child, data);
            }
            error = !ret;
            if (mph->isAttributeMember()) {
            } else if (!error) {
                child = child->NextSiblingElement();
            }

            if (error) {
                if (mph->isAttributeMember()) {
                    // no problem
                    continue;
                } else {
                    // error
                    return false;
                }
            }
        }
        return true;
    }

};

template<class T>
class GenericTiXmlBinding : public TiXmlBinding<T> {
public:
    virtual bool fromXml(TiXmlElement const& elem, T* data) const {
        TiXmlNode* node = elem.FirstChild();
        TiXmlText* nodedata = node->ToText();
        ConvertFromString(nodedata->Value(), data);
        return true;
    }

};

template<class T, class VecT>
class StlContainerTiXmlBinding : public TiXmlBinding<VecT> {
public:
    virtual bool fromXml(TiXmlElement const& elem, VecT* data) const {
        data->clear();
        TiXmlElement const* child;
        child = elem.FirstChildElement();
        while (child) {
            T* value = new T();
            TiXmlBinding<T> const* binding = GetTiXmlBinding(*value,  Identity<T>());
            bool ret = binding->fromXml(*child, value);
            data->push_back(*value);
            if (! ret) {
                return false;
            }
            child = child->NextSiblingElement();
        }
        return true;
    }

};

template<class T>
TiXmlBinding<T> const* GetTiXmlBinding(T const&, IdentityBase);

TiXmlBinding<float> const* GetTiXmlBinding(float const&, IdentityBase);
TiXmlBinding<std::string> const* GetTiXmlBinding(std::string const&, IdentityBase);

template<class T, class VecT>
TiXmlBinding<VecT> const* GetTiXmlBinding(std::vector<T> const&, Identity<VecT>) {
    static StlContainerTiXmlBinding<T, VecT> binding;
    return &binding;
}
