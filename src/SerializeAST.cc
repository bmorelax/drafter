//
//  SerializeAST.cc
//  drafter
//
//  Created by Pavan Kumar Sunkara on 18/01/15.
//  Copyright (c) 2015 Apiary Inc. All rights reserved.
//

#include "StringUtility.h"
#include "SerializeAST.h"

#include <tr1/functional>

using namespace drafter;

using snowcrash::AssetRole;
using snowcrash::BodyExampleAssetRole;
using snowcrash::BodySchemaAssetRole;
using snowcrash::Element;
using snowcrash::Elements;
using snowcrash::KeyValuePair;
using snowcrash::Metadata;
using snowcrash::MetadataCollection;
using snowcrash::Header;
using snowcrash::Reference;
using snowcrash::Asset;
using snowcrash::AssetRole;
using snowcrash::DataStructure;
using snowcrash::Payload;
using snowcrash::Headers;
using snowcrash::Parameters;
using snowcrash::Parameter;
using snowcrash::Values;
using snowcrash::TransactionExample;
using snowcrash::TransactionExamples;
using snowcrash::Responses;
using snowcrash::Requests;
using snowcrash::Action;
using snowcrash::Actions;
using snowcrash::Resource;
using snowcrash::Blueprint;

template<typename T, typename S = sos::Array>
struct CollectionPushWrapper {
    typedef typename T::const_iterator iterator_type;
    typedef typename T::value_type value_type;

    template<typename Functor>
    void operator()(const T& collection, S& target, Functor &wrapper) const {
        for( iterator_type it = collection.begin() ; it != collection.end() ; ++it ) {
            target.push(wrapper(*it));
        }
    }

};

sos::Object WrapValue(const mson::Value& value)
{
    sos::Object valueObject;

    // Literal
    valueObject.set(SerializeKey::Literal, sos::String(value.literal));

    // Variable
    valueObject.set(SerializeKey::Variable, sos::Boolean(value.variable));

    return valueObject;
}

sos::Object WrapSymbol(const mson::Symbol& symbol)
{
    sos::Object symbolObject;

    // Literal
    symbolObject.set(SerializeKey::Literal, sos::String(symbol.literal));

    // Variable
    symbolObject.set(SerializeKey::Variable, sos::Boolean(symbol.variable));

    return symbolObject;
}

sos::Base WrapTypeName(const mson::TypeName& typeName)
{
    if (typeName.empty()) {
        return sos::Null();
    }

    if (typeName.base != mson::UndefinedTypeName) {

        std::string baseTypeName;

        switch (typeName.base) {

            case mson::BooleanTypeName:
                baseTypeName = "boolean";
                break;

            case mson::StringTypeName:
                baseTypeName = "string";
                break;

            case mson::NumberTypeName:
                baseTypeName = "number";
                break;

            case mson::ArrayTypeName:
                baseTypeName = "array";
                break;

            case mson::EnumTypeName:
                baseTypeName = "enum";
                break;

            case mson::ObjectTypeName:
                baseTypeName = "object";
                break;

            default:
                break;
        }

        return sos::String(baseTypeName);
    }

    return WrapSymbol(typeName.symbol);
}

sos::Object WrapTypeSpecification(const mson::TypeSpecification& typeSpecification)
{
    sos::Object typeSpecificationObject;

    // Name
    typeSpecificationObject.set(SerializeKey::Name, WrapTypeName(typeSpecification.name));

    // Nested Types
    sos::Array nestedTypes;
    CollectionPushWrapper<mson::TypeNames>()(typeSpecification.nestedTypes, nestedTypes, WrapTypeName);
    typeSpecificationObject.set(SerializeKey::NestedTypes, nestedTypes);

    return typeSpecificationObject;
}

sos::Array WrapTypeAttributes(const mson::TypeAttributes& typeAttributes)
{
    sos::Array typeAttributesArray;

    if (typeAttributes & mson::RequiredTypeAttribute) {
        typeAttributesArray.push(sos::String("required"));
    }
    else if (typeAttributes & mson::OptionalTypeAttribute) {
        typeAttributesArray.push(sos::String("optional"));
    }
    else if (typeAttributes & mson::DefaultTypeAttribute) {
        typeAttributesArray.push(sos::String("default"));
    }
    else if (typeAttributes & mson::SampleTypeAttribute) {
        typeAttributesArray.push(sos::String("sample"));
    }
    else if (typeAttributes & mson::FixedTypeAttribute) {
        typeAttributesArray.push(sos::String("fixed"));
    }

    return typeAttributesArray;
}

sos::Object WrapTypeDefinition(const mson::TypeDefinition& typeDefinition)
{
    sos::Object typeDefinitionObject;

    // Type Specification
    typeDefinitionObject.set(SerializeKey::TypeSpecification, WrapTypeSpecification(typeDefinition.typeSpecification));

    // Type Attributes
    typeDefinitionObject.set(SerializeKey::Attributes, WrapTypeAttributes(typeDefinition.attributes));

    return typeDefinitionObject;
}

sos::Object WrapValueDefinition(const mson::ValueDefinition& valueDefinition)
{
    sos::Object valueDefinitionObject;

    // Values
    sos::Array values;
    CollectionPushWrapper<mson::Values>()(valueDefinition.values, values, WrapValue);
    valueDefinitionObject.set(SerializeKey::Values, values);

    // Type Definition
    valueDefinitionObject.set(SerializeKey::TypeDefinition, WrapTypeDefinition(valueDefinition.typeDefinition));

    return valueDefinitionObject;
}

sos::Object WrapPropertyName(const mson::PropertyName& propertyName)
{
    sos::Object propertyNameObject;

    if (!propertyName.literal.empty()) {
        propertyNameObject.set(SerializeKey::Literal, sos::String(propertyName.literal));
    }
    else if (!propertyName.variable.empty()) {
        propertyNameObject.set(SerializeKey::Variable, WrapValueDefinition(propertyName.variable));
    }

    return propertyNameObject;
}

// Forward declarations
sos::Array WrapTypeSections(const mson::TypeSections& typeSections);
sos::Array WrapMSONElements(const mson::Elements& elements);

sos::Object WrapPropertyMember(const mson::PropertyMember& propertyMember)
{
    sos::Object propertyMemberObject;

    // Name
    propertyMemberObject.set(SerializeKey::Name, WrapPropertyName(propertyMember.name));

    // Description
    propertyMemberObject.set(SerializeKey::Description, sos::String(propertyMember.description));

    // Value Definition
    propertyMemberObject.set(SerializeKey::ValueDefinition, WrapValueDefinition(propertyMember.valueDefinition));

    // Type Sections
    propertyMemberObject.set(SerializeKey::Sections, WrapTypeSections(propertyMember.sections));

    return propertyMemberObject;
}

sos::Object WrapValueMember(const mson::ValueMember& valueMember)
{
    sos::Object valueMemberObject;

    // Description
    valueMemberObject.set(SerializeKey::Description, sos::String(valueMember.description));

    // Value Definition
    valueMemberObject.set(SerializeKey::ValueDefinition, WrapValueDefinition(valueMember.valueDefinition));

    // Type Sections
    valueMemberObject.set(SerializeKey::Sections, WrapTypeSections(valueMember.sections));

    return valueMemberObject;
}

sos::Object WrapMixin(const mson::Mixin& mixin)
{
    return WrapTypeDefinition(mixin);
}

sos::Array WrapOneOf(const mson::OneOf& oneOf)
{
    return WrapMSONElements(oneOf);
}

sos::Object WrapMSONElement(const mson::Element& element)
{
    sos::Object elementObject;
    std::string klass;

    switch (element.klass) {

        case mson::Element::PropertyClass:
        {
            klass = "property";
            elementObject.set(SerializeKey::Content, WrapPropertyMember(element.content.property));
            break;
        }

        case mson::Element::ValueClass:
        {
            klass = "value";
            elementObject.set(SerializeKey::Content, WrapValueMember(element.content.value));
            break;
        }

        case mson::Element::MixinClass:
        {
            klass = "mixin";
            elementObject.set(SerializeKey::Content, WrapMixin(element.content.mixin));
            break;
        }

        case mson::Element::OneOfClass:
        {
            klass = "oneOf";
            elementObject.set(SerializeKey::Content, WrapOneOf(element.content.oneOf()));
            break;
        }

        case mson::Element::GroupClass:
        {
            klass = "group";
            elementObject.set(SerializeKey::Content, WrapMSONElements(element.content.elements()));
            break;
        }

        default:
            break;
    }

    elementObject.set(SerializeKey::Class, sos::String(klass));

    return elementObject;
}

sos::Array WrapMSONElements(const mson::Elements& elements)
{
    sos::Array elementsArray;

    CollectionPushWrapper<mson::Elements>()(elements, elementsArray, WrapMSONElement);

    return elementsArray;
}

sos::Array WrapTypeSections(const mson::TypeSections& sections)
{
    sos::Array sectionsArray;

    for (mson::TypeSections::const_iterator it = sections.begin(); it != sections.end(); ++it) {

        sos::Object section;

        // Class
        std::string klass;

        switch (it->klass) {
            case mson::TypeSection::BlockDescriptionClass:
                klass = "blockDescription";
                break;

            case mson::TypeSection::MemberTypeClass:
                klass = "memberType";
                break;

            case mson::TypeSection::SampleClass:
                klass = "sample";
                break;

            case mson::TypeSection::DefaultClass:
                klass = "default";
                break;

            default:
                break;
        }

        section.set(SerializeKey::Class, sos::String(klass));

        // Content
        if (!it->content.description.empty()) {
            section.set(SerializeKey::Content, sos::String(it->content.description));
        }
        else if (!it->content.value.empty()) {
            section.set(SerializeKey::Content, sos::String(it->content.value));
        }
        else if (!it->content.elements().empty()) {
            section.set(SerializeKey::Content, WrapMSONElements(it->content.elements()));
        }

        sectionsArray.push(section);
    }

    return sectionsArray;
}

sos::Object WrapNamedType(const mson::NamedType& namedType)
{
    sos::Object namedTypeObject;

    // Name
    namedTypeObject.set(SerializeKey::Name, WrapTypeName(namedType.name));

    // Type Definition
    namedTypeObject.set(SerializeKey::TypeDefinition, WrapTypeDefinition(namedType.typeDefinition));

    // Type Sections
    namedTypeObject.set(SerializeKey::Sections, WrapTypeSections(namedType.sections));

    return namedTypeObject;
}

sos::String WrapAssetRole(const AssetRole& role)
{
    std::string str;

    switch (role) {
        case BodyExampleAssetRole:
            str = "bodyExample";
            break;

        case BodySchemaAssetRole:
            str = "bodySchema";
            break;

        default:
            break;
    }

    return sos::String(str);
}

sos::String WrapElementClass(const Element::Class& element)
{
    std::string str;

    switch (element) {
        case Element::CategoryElement:
            str = "category";
            break;

        case Element::CopyElement:
            str = "copy";
            break;

        case Element::ResourceElement:
            str = "resource";
            break;

        case Element::DataStructureElement:
            str = "dataStructure";
            break;

        case Element::AssetElement:
            str = "asset";
            break;

        default:
            break;
    }

    return sos::String(str);
}

sos::Object WrapKeyValue(const KeyValuePair& keyValue)
{
    sos::Object keyValueObject;

    // Name
    keyValueObject.set(SerializeKey::Name, sos::String(keyValue.first));

    // Value
    keyValueObject.set(SerializeKey::Value, sos::String(keyValue.second));

    return keyValueObject;
}

sos::Object WrapMetadata(const Metadata& metadata)
{
    return WrapKeyValue(metadata);
}

sos::Object WrapHeader(const Header& header)
{
    return WrapKeyValue(header);
}

sos::Object WrapReference(const Reference& reference)
{
    sos::Object referenceObject;

    // Id
    referenceObject.set(SerializeKey::Id, sos::String(reference.id));

    return referenceObject;
}

sos::Object WrapAsset(const Asset& asset, const AssetRole& role)
{
    sos::Object assetObject;

    // Element
    assetObject.set(SerializeKey::Element, WrapElementClass(Element::AssetElement));

    // Attributes
    sos::Object attributes;

    /// Role
    attributes.set(SerializeKey::Role, WrapAssetRole(role));

    assetObject.set(SerializeKey::Attributes, attributes);

    // Content
    assetObject.set(SerializeKey::Content, sos::String(asset));

    return assetObject;
}

sos::Object WrapDataStructure(const DataStructure& dataStructure)
{
    sos::Object dataStructureObject;

    // Element
    dataStructureObject.set(SerializeKey::Element, WrapElementClass(Element::DataStructureElement));

    // Name
    dataStructureObject.set(SerializeKey::Name, WrapTypeName(dataStructure.name));

    // Type Definition
    dataStructureObject.set(SerializeKey::TypeDefinition, WrapTypeDefinition(dataStructure.typeDefinition));

    // Type Sections
    dataStructureObject.set(SerializeKey::Sections, WrapTypeSections(dataStructure.sections));

    return dataStructureObject;
}

sos::Object WrapPayload(const Payload& payload)
{
    sos::Object payloadObject;

    // Reference
    if (!payload.reference.id.empty()) {
        payloadObject.set(SerializeKey::Reference, WrapReference(payload.reference));
    }

    // Name
    payloadObject.set(SerializeKey::Name, sos::String(payload.name));

    // Description
    payloadObject.set(SerializeKey::Description, sos::String(payload.description));

    // Headers
    sos::Array headers;
    CollectionPushWrapper<Headers>()(payload.headers, headers, WrapHeader);
    payloadObject.set(SerializeKey::Headers, headers);

    // Body
    payloadObject.set(SerializeKey::Body, sos::String(payload.body));

    // Schema
    payloadObject.set(SerializeKey::Schema, sos::String(payload.schema));

    // Content
    sos::Array content;

    /// Attributes
    if (!payload.attributes.empty()) {
        content.push(WrapDataStructure(payload.attributes));
    }

    /// Asset 'bodyExample'
    if (!payload.body.empty()) {
        content.push(WrapAsset(payload.body, BodyExampleAssetRole));
    }

    /// Asset 'bodySchema'
    if (!payload.schema.empty()) {
        content.push(WrapAsset(payload.schema, BodySchemaAssetRole));
    }

    payloadObject.set(SerializeKey::Content, content);

    return payloadObject;
}

sos::Object WrapParameter(const Parameter& parameter)
{

    sos::Object parameterObject;

    // Name
    parameterObject.set(SerializeKey::Name, sos::String(parameter.name));

    // Description
    parameterObject.set(SerializeKey::Description, sos::String(parameter.description));

    // Type
    parameterObject.set(SerializeKey::Type, sos::String(parameter.type));

    // Use
    parameterObject.set(SerializeKey::Required, sos::Boolean(parameter.use != snowcrash::OptionalParameterUse));

    // Default Value
    parameterObject.set(SerializeKey::Default, sos::String(parameter.defaultValue));

    // Example Value
    parameterObject.set(SerializeKey::Example, sos::String(parameter.exampleValue));

    // Values
    sos::Array values;

    for (Values::const_iterator it = parameter.values.begin(); it != parameter.values.end(); ++it) {

        sos::Object value;

        value.set(SerializeKey::Value, sos::String(it->c_str()));

        values.push(value);
    }

    parameterObject.set(SerializeKey::Values, values);

    return parameterObject;
}

sos::Array WrapParameters(const Parameters& parameters)
{
    sos::Array parametersArray;
    CollectionPushWrapper<Parameters>()(parameters, parametersArray, WrapParameter);

    return parametersArray;
}

sos::Object WrapTransactionExample(const TransactionExample& example)
{
    sos::Object exampleObject;

    // Name
    exampleObject.set(SerializeKey::Name, sos::String(example.name));

    // Description
    exampleObject.set(SerializeKey::Description, sos::String(example.description));

    // Requests
    sos::Array requests;
    CollectionPushWrapper<Requests>()(example.requests, requests, WrapPayload);
    exampleObject.set(SerializeKey::Requests, requests);

    // Responses
    sos::Array responses;
    CollectionPushWrapper<Responses>()(example.responses, responses, WrapPayload);
    exampleObject.set(SerializeKey::Responses, responses);

    return exampleObject;
}

sos::Object WrapAction(const Action& action)
{
    sos::Object actionObject;

    // Name
    actionObject.set(SerializeKey::Name, sos::String(action.name));

    // Description
    actionObject.set(SerializeKey::Description, sos::String(action.description));

    // HTTP Method
    actionObject.set(SerializeKey::Method, sos::String(action.method));

    // Parameters
    sos::Array parameters;
    CollectionPushWrapper<Parameters>()(action.parameters, parameters, WrapParameter);
    actionObject.set(SerializeKey::Parameters, parameters);

    // Content
    sos::Array content;

    if (!action.attributes.empty()) {
        content.push(WrapDataStructure(action.attributes));
    }

    actionObject.set(SerializeKey::Content, content);

    // Transaction Examples
    sos::Array transactionExamples;
    CollectionPushWrapper<TransactionExamples>()(action.examples, transactionExamples, WrapTransactionExample);
    actionObject.set(SerializeKey::Examples, transactionExamples);

    return actionObject;
}

sos::Object WrapResource(const Resource& resource)
{
    sos::Object resourceObject;

    // Element
    resourceObject.set(SerializeKey::Element, WrapElementClass(Element::ResourceElement));

    // Name
    resourceObject.set(SerializeKey::Name, sos::String(resource.name));

    // Description
    resourceObject.set(SerializeKey::Description, sos::String(resource.description));

    // URI Template
    resourceObject.set(SerializeKey::URITemplate, sos::String(resource.uriTemplate));

    // Model
    sos::Object model = (resource.model.name.empty() ? sos::Object() : WrapPayload(resource.model));
    resourceObject.set(SerializeKey::Model, model);

    // Parameters
    sos::Array parameters;
    CollectionPushWrapper<Parameters>()(resource.parameters, parameters, WrapParameter);
    resourceObject.set(SerializeKey::Parameters, parameters);

    // Actions
    sos::Array actions;
    CollectionPushWrapper<Actions>()(resource.actions, actions, WrapAction);
    resourceObject.set(SerializeKey::Actions, actions);

    // Content
    sos::Array content;

    if (!resource.attributes.empty()) {
        content.push(WrapDataStructure(resource.attributes));
    }

    resourceObject.set(SerializeKey::Content, content);

    return resourceObject;
}

sos::Object WrapResourceGroup(const Element& resourceGroup)
{
    sos::Object resourceGroupObject;

    // Name
    resourceGroupObject.set(SerializeKey::Name, sos::String(resourceGroup.attributes.name));

    // Description && Resources
    std::string description;
    sos::Array resources;

    for (Elements::const_iterator it = resourceGroup.content.elements().begin();
         it != resourceGroup.content.elements().end();
         ++it) {

        if (it->element == Element::ResourceElement) {
            resources.push(WrapResource(it->content.resource));
        }
        else if (it->element == Element::CopyElement) {

            if (!description.empty()) {
                snowcrash::TwoNewLines(description);
            }

            description += it->content.copy;
        }
    }

    resourceGroupObject.set(SerializeKey::Description, sos::String(description));
    resourceGroupObject.set(SerializeKey::Resources, resources);

    return resourceGroupObject;
}

sos::Object WrapElement(const Element& element)
{
    sos::Object elementObject;

    elementObject.set(SerializeKey::Element, WrapElementClass(element.element));

    if (!element.attributes.name.empty()) {

        sos::Object attributes;

        attributes.set(SerializeKey::Name, sos::String(element.attributes.name));
        elementObject.set(SerializeKey::Attributes, attributes);
    }

    switch (element.element) {
        case Element::CopyElement:
        {
            elementObject.set(SerializeKey::Content, sos::String(element.content.copy));
            break;
        }

        case Element::CategoryElement:
        {
            sos::Array content;
            CollectionPushWrapper<Elements>()(element.content.elements(), content, WrapElement);
            elementObject.set(SerializeKey::Content, content);
            break;
        }

        case Element::DataStructureElement:
        {
            return WrapDataStructure(element.content.dataStructure);
        }

        case Element::ResourceElement:
        {
            return WrapResource(element.content.resource);
        }

        default:
            break;
    }

    return elementObject;
}



sos::Object drafter::WrapBlueprint(const Blueprint& blueprint)
{
    sos::Object blueprintObject;

    // Version
    blueprintObject.set(SerializeKey::ASTVersion, sos::String(AST_SERIALIZATION_VERSION));

    // Metadata
    sos::Array metadata;

    CollectionPushWrapper<MetadataCollection>()(blueprint.metadata, metadata, WrapKeyValue);

    blueprintObject.set(SerializeKey::Metadata, metadata);

    // Name
    blueprintObject.set(SerializeKey::Name, sos::String(blueprint.name));

    // Description
    blueprintObject.set(SerializeKey::Description, sos::String(blueprint.description));

    // Element
    blueprintObject.set(SerializeKey::Element, WrapElementClass(blueprint.element));

    // Resource Groups
    sos::Array resourceGroups;

    for (Elements::const_iterator it = blueprint.content.elements().begin();
         it != blueprint.content.elements().end();
         ++it) {

        if (it->element == Element::CategoryElement &&
            it->category == Element::ResourceGroupCategory) {

            resourceGroups.push(WrapResourceGroup(*it));
        }
    }

    blueprintObject.set(SerializeKey::ResourceGroups, resourceGroups);

    // Content
    sos::Array content;
    CollectionPushWrapper<Elements>()(blueprint.content.elements(), content, WrapElement);
    blueprintObject.set(SerializeKey::Content, content);

    return blueprintObject;
}
