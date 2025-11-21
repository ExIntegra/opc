/**
 * @file opcuaSettings.c
 * @brief OPC UA address space helpers, custom types and data source callbacks.
 *
 * This module wires the C data structures of the process model to the
 * open62541 OPC UA server address space. It provides:
 *
 *   - DataSource callbacks for Double and UInt32 values
 *     (readDoubleDS, writeDoubleDS, readUInt32DS, writeUInt32DS) to expose
 *     struct fields as OPC UA variables with custom validation and logging.
 *
 *   - Utility functions to locate child variable nodes by browse name and
 *     bind them to C fields using UA_DataSource:
 *       * find_child_var()
 *       * attach_child_double()
 *       * attach_child_UInt32()
 *
 *   - Registration of custom ObjectTypes used by the application:
 *       * SensorType
 *       * ReactorType
 *       * ValveHandleControlType
 *       * MathModelType
 *
 *   - Factory helpers that create instances of these types in the server
 *     address space and connect them to the corresponding C structures:
 *       * opc_ua_create_sensor_instance()
 *       * opc_ua_create_reactor_instance()
 *       * opc_ua_create_valve_handle_control()
 *       * opc_ua_create_math_model_instance()
 *       * opc_ua_create_cell_folder()
 */

#include <stdio.h>
#include <math.h>
#include "opcuaSettings.h"
#include "types.h"
#include <open62541/plugin/log_stdout.h>
#include <open62541/types.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

 /**
  * @brief DataSource read callback for Double variables.
  *
  * Reads the Double value from nodeContext, fills UA_DataValue
  * with scalar value and timestamps, and performs basic range
  * / index checks.
  */
static UA_StatusCode readDoubleDS(UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange* range,
    UA_DataValue* out) {

    (void)sessionId;
    (void)sessionContext;

    UA_DataValue_init(out);

    if (!nodeContext) {
        out->status = UA_STATUSCODE_BADINTERNALERROR;
        out->hasStatus = true;
        return out->status;
    }

    if (range && range->dimensionsSize > 0) {
        out->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        out->hasStatus = true;
        return out->status;
    }

    const UA_Double v = *(const UA_Double*)nodeContext;

    UA_StatusCode rv =
        UA_Variant_setScalarCopy(&out->value, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
    if (rv != UA_STATUSCODE_GOOD) {
        out->status = rv;
        out->hasStatus = true;
        return rv;
    }

    out->hasValue = true;

    if (includeSourceTimeStamp) {
        out->sourceTimestamp = UA_DateTime_now();
        out->hasSourceTimestamp = true;
    }

    out->serverTimestamp = UA_DateTime_now();
    out->hasServerTimestamp = true;

    out->status = UA_STATUSCODE_GOOD;
    out->hasStatus = true;

    return UA_STATUSCODE_GOOD;
}

/**
 * @brief DataSource write callback for Double variables.
 *
 * Validates the incoming value (type, rank, finite), writes it
 * into nodeContext, and logs the new value together with the
 * node's browse name or numeric NodeId.
 */
static UA_StatusCode writeDoubleDS(UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* data) {

    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;

    if (!nodeContext)
        return UA_STATUSCODE_BADINTERNALERROR;

    if (!data || !data->hasValue)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    if (range && range->dimensionsSize > 0)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;

    if (data->value.type != &UA_TYPES[UA_TYPES_DOUBLE] ||
        data->value.data == NULL ||
        data->value.arrayLength != 0 ||
        data->value.arrayDimensionsSize != 0)
        return UA_STATUSCODE_BADTYPEMISMATCH;

    const UA_Double v = *(const UA_Double*)data->value.data;
    if (!isfinite(v))
        return UA_STATUSCODE_BADOUTOFRANGE;

    *(UA_Double*)nodeContext = v;
    if (server && nodeId) {
        UA_QualifiedName bn;
        UA_StatusCode rc = UA_Server_readBrowseName(server, *nodeId, &bn);
        if (rc == UA_STATUSCODE_GOOD) {
            printf("writeDoubleDS: %.*s = %.3f\n",
                (int)bn.name.length, bn.name.data, v);
            UA_QualifiedName_clear(&bn);
        }
        else {
            if (nodeId->identifierType == UA_NODEIDTYPE_NUMERIC) {
                printf("writeDoubleDS: ns=%u;i=%u = %.3f\n",
                    nodeId->namespaceIndex,
                    nodeId->identifier.numeric,
                    v);
            }
            else {
                printf("writeDoubleDS: <unknown node> = % .3f\n", v);
            }
        }
    }
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief DataSource write callback for UInt32 variables.
 *
 * Validates the incoming UInt32 value and writes it into
 * nodeContext. No logging is performed here.
 */
static UA_StatusCode writeUInt32DS(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* data) {

    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;

    if (!nodeContext)
        return UA_STATUSCODE_BADINTERNALERROR;

    if (!data || !data->hasValue)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    if (range && range->dimensionsSize > 0)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;

    if (data->value.type != &UA_TYPES[UA_TYPES_UINT32] ||
        data->value.data == NULL ||
        data->value.arrayLength != 0 ||
        data->value.arrayDimensionsSize != 0)
        return UA_STATUSCODE_BADTYPEMISMATCH;

    const UA_UInt32 v = *(const UA_UInt32*)data->value.data;
    *(UA_UInt32*)nodeContext = v;
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief DataSource read callback for UInt32 variables.
 *
 * Reads the UInt32 value from nodeContext, returns it as a
 * scalar Variant in UA_DataValue and sets timestamps.
 */
static UA_StatusCode readUInt32DS(UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange* range,
    UA_DataValue* out) {

    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;

    UA_DataValue_init(out);

    if (!nodeContext) {
        out->status = UA_STATUSCODE_BADINTERNALERROR;
        out->hasStatus = true;
        return out->status;
    }

    if (range && range->dimensionsSize > 0) {
        out->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        out->hasStatus = true;
        return out->status;
    }

    const UA_UInt32 v = *(const UA_UInt32*)nodeContext;

    UA_StatusCode rv = UA_Variant_setScalarCopy(&out->value, &v, &UA_TYPES[UA_TYPES_UINT32]);
    if (rv != UA_STATUSCODE_GOOD) {
        out->status = rv;
        out->hasStatus = true;
        return rv;
    }

    out->hasValue = true;

    if (includeSourceTimeStamp) {
        out->sourceTimestamp = UA_DateTime_now();
        out->hasSourceTimestamp = true;
    }

    out->serverTimestamp = UA_DateTime_now();
    out->hasServerTimestamp = true;

    out->status = UA_STATUSCODE_GOOD;
    out->hasStatus = true;
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Finds a child variable node by browse name under a parent node.
 *
 * Uses TranslateBrowsePathsToNodeIds with HasComponent to resolve a
 * single browseName relative to parent and returns the resulting NodeId
 * in out.
 */
static UA_StatusCode find_child_var(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    UA_NodeId* out) {

    UA_RelativePathElement rpe;
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    rpe.isInverse = false;
    rpe.includeSubtypes = false;
    rpe.targetName = UA_QUALIFIEDNAME(1, (char*)browseName);

    UA_BrowsePath bp;
    UA_BrowsePath_init(&bp);
    bp.startingNode = parent;
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;

    UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(server, &bp);

    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1) {
        UA_BrowsePathResult_clear(&bpr);
        return UA_STATUSCODE_BADNOTFOUND;
    }

    UA_StatusCode c = UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, out);

    UA_BrowsePathResult_clear(&bpr);
    return (c == UA_STATUSCODE_GOOD) ? UA_STATUSCODE_GOOD : c;
}

/**
 * @brief Binds a Double field to a child variable node and installs DataSource.
 *
 * Resolves the child variable under parent by browse name, sets the
 * node context pointer to ptrToField, and attaches readDoubleDS /
 * writeDoubleDS as its DataSource.
 */
static UA_StatusCode attach_child_double(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);

    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    UA_DataSource ds;
    ds.read = readDoubleDS;
    ds.write = writeDoubleDS;

    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL);
        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Binds a UInt32 field to a child variable node and installs DataSource.
 *
 * Resolves the child variable under parent by browse name, sets the
 * node context pointer to ptrToField, and attaches readUInt32DS /
 * writeUInt32DS as its DataSource.
 */
static UA_StatusCode attach_child_UInt32(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    UA_DataSource ds;
    ds.read = readUInt32DS;
    ds.write = writeUInt32DS;

    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL);

        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Adds ModellingRule Mandatory reference to a variable node.
 *
 * Marks the given node as mandatory component of its type by
 * adding HasModellingRule -> ModellingRule_Mandatory reference.
 */
static UA_StatusCode add_reference_mandatory(UA_Server* server, UA_NodeId nodeId) {
    return UA_Server_addReference(server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
        true);
}

UA_NodeId sensorTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1002 } };
UA_NodeId reactorTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1004 } };
UA_NodeId valveHandleControlType = { 1, UA_NODEIDTYPE_NUMERIC, { 1005 } };
UA_NodeId mathModelTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1006 } };

/**
 * @brief Declares the ValveHandleControlType ObjectType in namespace 1.
 *
 * Creates a custom ObjectType with a mandatory Double variable
 * MANUAL_OUTPUT to represent manual valve position (0–100 %).
 */
UA_NodeId addValveHandleControlType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ValveHandleControlType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1005),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "ValveHandleControlType"),
        varAttr, NULL, &valveHandleControlType);

    UA_VariableAttributes manualOutputAttr = UA_VariableAttributes_default;
    manualOutputAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MANUAL_OUTPUT");
    manualOutputAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    manualOutputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId manualOutputId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveHandleControlType,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "MANUAL_OUTPUT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        manualOutputAttr, NULL, &manualOutputId);
    add_reference_mandatory(server, manualOutputId);
    return valveHandleControlType;
}

/**
 * @brief Declares the ReactorType ObjectType in namespace 1.
 *
 * Creates a custom ObjectType with a mandatory Double variable
 * REACTOR_VOLUME to represent reactor volume.
 */
UA_NodeId addReactorType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ReactorType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1004),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "ReactorType"),
        varAttr, NULL, &reactorTypeId);

    UA_VariableAttributes reactorAttr = UA_VariableAttributes_default;
    reactorAttr.displayName = UA_LOCALIZEDTEXT("en-US", "REACTOR_VOLUME");
    reactorAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    reactorAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId reactorId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, reactorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "REACTOR_VOLUME"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        reactorAttr, NULL, &reactorId);
    add_reference_mandatory(server, reactorId);
    return reactorTypeId;
}

/**
 * @brief Declares the MathModelType ObjectType in namespace 1.
 *
 * Creates a custom ObjectType for kinetic model configuration with
 * variables: SUBSTANCE_ID, K01, K02, EA1, EA2.
 */
UA_NodeId addMathModelType(UA_Server* server) {
    UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "MathModelType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1006),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "MathModelType"),
        attr, NULL, &mathModelTypeId);

    UA_VariableAttributes idAttr = UA_VariableAttributes_default;
    idAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SUBSTANCE_ID");
    idAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
    idAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId idNode;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, mathModelTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SUBSTANCE_ID"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        idAttr, NULL, &idNode);
    add_reference_mandatory(server, idNode);

    UA_VariableAttributes k1Attr = UA_VariableAttributes_default;
    k1Attr.displayName = UA_LOCALIZEDTEXT("en-US", "K01");
    k1Attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    k1Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId k1Id;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, mathModelTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "K01"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        k1Attr, NULL, &k1Id);
    add_reference_mandatory(server, k1Id);

    UA_VariableAttributes k2Attr = UA_VariableAttributes_default;
    k2Attr.displayName = UA_LOCALIZEDTEXT("en-US", "K02");
    k2Attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    k2Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId k2Id;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, mathModelTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "K02"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        k2Attr, NULL, &k2Id);
    add_reference_mandatory(server, k2Id);

    UA_VariableAttributes E1Attr = UA_VariableAttributes_default;
    E1Attr.displayName = UA_LOCALIZEDTEXT("en-US", "EA1");
    E1Attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    E1Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId E1Id;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, mathModelTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "EA1"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        E1Attr, NULL, &E1Id);
    add_reference_mandatory(server, E1Id);

    UA_VariableAttributes E2Attr = UA_VariableAttributes_default;
    E2Attr.displayName = UA_LOCALIZEDTEXT("en-US", "EA2");
    E2Attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    E2Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId E2Id;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, mathModelTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "EA2"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        E2Attr, NULL, &E2Id);
    add_reference_mandatory(server, E2Id);
    return mathModelTypeId;
}

/**
 * @brief Declares the SensorType ObjectType in namespace 1.
 *
 * Creates a custom ObjectType with a mandatory Double variable
 * PROCESS_VALUE to represent the measured value.
 */
UA_NodeId addSensorType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SensorType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1002),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "SensorType"),
        varAttr, NULL, &sensorTypeId);

    UA_VariableAttributes pvAttr = UA_VariableAttributes_default;
    pvAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PROCESS_VALUE");
    pvAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    pvAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId pvId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "PROCESS_VALUE"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        pvAttr, NULL, &pvId);
    add_reference_mandatory(server, pvId);
    return sensorTypeId;
}

/**
 * @brief Creates a ValveHandleControl instance object and binds MANUAL_OUTPUT.
 *
 * Adds an Object of type ValveHandleControlType under parentFolder,
 * stores its NodeId into valveHandleControl->objId and attaches the
 * MANUAL_OUTPUT variable to valveHandleControl->manualoutput.
 */
UA_StatusCode opc_ua_create_valve_handle_control(UA_Server* server,
    UA_NodeId parentFolder, const char* valveHandleControlName, ValveHandleControl* valveHandleControl) {
    UA_NodeId valveHandleControlObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)valveHandleControlName),
        valveHandleControlType,
        UA_ObjectAttributes_default, NULL, &valveHandleControlObjId);
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object valve handle control %s\n", valveHandleControlName);
        return rc;
    }
    else {
        printf("Valve Handle Control %s created successfully\n", valveHandleControlName);
    }
    valveHandleControl->objId = valveHandleControlObjId;
    rc = attach_child_double(server, valveHandleControlObjId, "MANUAL_OUTPUT", &valveHandleControl->manualoutput); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Creates a Reactor instance object and binds REACTOR_VOLUME.
 *
 * Adds an Object of type ReactorType under parentFolder, stores its
 * NodeId into reactor->objId and attaches the REACTOR_VOLUME variable
 * to reactor->volume.
 */
UA_StatusCode opc_ua_create_reactor_instance(UA_Server* server,
    UA_NodeId parentFolder, const char* reactorName, Reactor* reactor) {
    UA_NodeId reactorObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)reactorName),
        reactorTypeId,
        UA_ObjectAttributes_default, NULL, &reactorObjId);
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object reactor %s\n", reactorName);
        return rc;
    }
    else {
        printf("Reactor %s created successfully\n", reactorName);
    }
    reactor->objId = reactorObjId;
    rc = attach_child_double(server, reactorObjId, "REACTOR_VOLUME", &reactor->volume); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Creates a Sensor instance object and binds PROCESS_VALUE.
 *
 * Adds an Object of type SensorType under parentFolder, stores its
 * NodeId into sensor->objId and attaches the PROCESS_VALUE variable
 * to sensor->pv.
 */
UA_StatusCode opc_ua_create_sensor_instance(UA_Server* server,
    UA_NodeId parentFolder, const char* sensorName,
    UA_Boolean enableAlarms, Sensor* sensor)
{
    (void)enableAlarms; /* alarms not used in this version */

    UA_NodeId sensorObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)sensorName),
        sensorTypeId,
        UA_ObjectAttributes_default, NULL, &sensorObjId);
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object sensor %s\n", sensorName);
        return rc;
    }
    else {
        printf("Sensor %s created successfully\n", sensorName);
    }

    sensor->objId = sensorObjId;

    rc = attach_child_double(server, sensorObjId, "PROCESS_VALUE", &sensor->pv); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Creates a MathModelType instance and binds configuration fields.
 *
 * Adds an Object of type MathModelType under parentFolder and binds
 * SUBSTANCE_ID, K01, K02, EA1, EA2 to the corresponding fields in
 * the ModelCtx structure.
 */
UA_StatusCode opc_ua_create_math_model_instance(UA_Server* server,
    UA_NodeId parentFolder, const char* name, ModelCtx* m)
{
    UA_NodeId objId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL, parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)name),
        mathModelTypeId,
        UA_ObjectAttributes_default, NULL, &objId);
    if (rc) return rc;

    rc = attach_child_UInt32(server, objId, "SUBSTANCE_ID", &m->substanceId); if (rc) return rc;
    rc = attach_child_double(server, objId, "K01", &m->cfg.k01); if (rc) return rc;
    rc = attach_child_double(server, objId, "K02", &m->cfg.k02); if (rc) return rc;
    rc = attach_child_double(server, objId, "EA1", &m->cfg.EA1); if (rc) return rc;
    rc = attach_child_double(server, objId, "EA2", &m->cfg.EA2); if (rc) return rc;

    return UA_STATUSCODE_GOOD;
}

/**
 * @brief Creates a top-level folder under Objects for grouping instances.
 *
 * Adds a FolderType object with the given name under ObjectsFolder
 * and returns its NodeId in outFolderId.
 */
UA_StatusCode opc_ua_create_cell_folder(UA_Server* server,
    const char* cellName,
    UA_NodeId* outFolderId) {
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)cellName);

    printf("create");
    return UA_Server_addObjectNode(server,
        UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)cellName),
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
        oAttr, NULL, outFolderId);
}
