//
//  Serialize.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/3/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SERIALIZE_H
#define SNOWCRASH_SERIALIZE_H

#include <string>
#include "BlueprintSourcemap.h"
#include "sos.h"

/** Version of API Blueprint AST serialization */
#define AST_SERIALIZATION_VERSION "2.1"

namespace snowcrash {

    /**
     *  AST entities serialization keys
     */
    struct SerializeKey {
        static const std::string ASTVersion;
        static const std::string Metadata;
        static const std::string Reference;
        static const std::string Id;
        static const std::string Name;
        static const std::string Description;
        static const std::string ResourceGroups;
        static const std::string Resources;
        static const std::string URI;
        static const std::string URITemplate;
        static const std::string Assets;
        static const std::string Actions;
        static const std::string Action;
        static const std::string Attributes;
        static const std::string Examples;
        static const std::string Transaction;
        static const std::string Method;
        static const std::string Requests;
        static const std::string Responses;
        static const std::string Body;
        static const std::string Schema;
        static const std::string Headers;
        static const std::string Model;
        static const std::string Value;
        static const std::string Parameters;
        static const std::string Type;
        static const std::string Required;
        static const std::string Default;
        static const std::string Example;
        static const std::string Values;

        static const std::string Source;
        static const std::string Resolved;

        static const std::string Literal;
        static const std::string Variable;
        static const std::string TypeDefinition;
        static const std::string TypeSpecification;
        static const std::string NestedTypes;
        static const std::string Sections;
        static const std::string Class;
        static const std::string Content;
        static const std::string ValueDefinition;
    };
}

#endif
