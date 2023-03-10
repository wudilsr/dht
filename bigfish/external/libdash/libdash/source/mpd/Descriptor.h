/*
 * Descriptor.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_

#include "config.h"

#include "IDescriptor.h"
#include "AbstractMPDElement.h"
#include "MSPR.h"

namespace dash
{
    namespace mpd
    {
        class Descriptor : public IDescriptor, public AbstractMPDElement
        {
            public:
                Descriptor          ();
                virtual ~Descriptor ();

                const std::string&      GetSchemeIdUri  () const;
                const std::string&      GetValue        () const;
                const std::vector<IMSPR *>& GetMSPR     () const;

                void    SetValue        (const std::string& value);
                void    SetSchemeIdUri  (const std::string& schemeIdUri);
                void    AddMSPR         (MSPR *mspr);

            private:
                std::string  schemeIdUri;
                std::string  value;
                std::vector<MSPR *>      mspr;
        };
    }
}

#endif /* DESCRIPTOR_H_ */
