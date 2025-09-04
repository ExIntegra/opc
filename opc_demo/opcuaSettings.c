#include "opcuaSettings.h"


static UA_StatusCode
readDoubleDS(UA_Server* server,
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

    /* ������� nodeContext � ���� double � �������������� ��������� */
    const double v = *(const double*)nodeContext;

    UA_StatusCode rv = UA_Variant_setScalarCopy(&out->value, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
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


/* ������� ��� ������ ������ � ����. ��������
 * � ������������ ���������� PID (kp, ki, kd, output, setPoint, processValue)*/
static UA_StatusCode
writeDoubleDS(UA_Server* server,
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

    /* ���������� �� ������, ����������� ����� nodeContext */
    *(double*)nodeContext = v;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
writeBoolDS(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* data) {
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
readBoolDS(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    UA_Boolean sourceTimeStamp, const UA_NumericRange* range,
    UA_DataValue* dataValue) {
    return UA_STATUSCODE_GOOD;
}

/*������ ��� ���������� ���������� �������� � �������� ��� � nodeId
 * ��� ��������� ������ � ���.
 * parent - NodeId �������� (������)
 * browseName - ��� �������� ���������� ��������
 * out - nodeId ���������� ���������� ��������*/
static UA_StatusCode
findChildVar(UA_Server* server, const UA_NodeId parent,
    const char* browseName, UA_NodeId* out) {

    //��������� ���, �� �������� ������ �����:
    UA_RelativePathElement rpe; //������� ��������� ���������
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT); //��� ����� �������� � ����������
    //���� ������ �� �������� � ����������.
    rpe.isInverse = false;
    rpe.includeSubtypes = false; //�� ��������� ������� ������
    //��� ����, � �������� ���� � ������.
    rpe.targetName = UA_QUALIFIEDNAME(1, (char*)browseName);
    //������� ��� � ��������� � �������� �������� �� BrowsePath.


    //�������� ������� ��  ���������� ��������� BrowsePath , ������� �� parent, �������� ������� rpe.
    UA_BrowsePath bp; //��������� ��������� Browsepath
    UA_BrowsePath_init(&bp);
    bp.startingNode = parent; //������ �������� �����
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;

    //���������� �����, � UA_BrowsePathResult brp �������� ����������� ��������� ���������� ������.
    //����� statrusCode - ������ ����� �������� � targetSize - ������� ����� �����.
    UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(server, &bp);

    //��������� ��������� �����.
    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1) {
        UA_BrowsePathResult_clear(&bpr);
        return UA_STATUSCODE_BADNOTFOUND;
    }

    //���� ������ ��������� ������ (ExpandedNodeId) � ��������� ��������� NodeId.
    *out = bpr.targets[0].targetId.nodeId;

    //������ ��������� ��������� ���������� */
    UA_BrowsePathResult_clear(&bpr);
    return UA_STATUSCODE_GOOD;
}

/*������� ��� ���������� nodeContext � ��������� �����
 *� ������� ������� findChildVar.
 *parent - ������������ ����
 *name - ��� �������� ����
 *pid - ��������� ��������� ���
 *field - ��������� ���������� � �������� ������������� nodeContext (KP, KI, ...)*/
static UA_StatusCode
attachChild(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    PIDControllerData* pid,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //������� ���� � �������� ��� � chilId ����� ���������:
    UA_StatusCode ret = findChildVar(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);

    // ������������� DataSource.
    UA_DataSource ds;
    ds.read = readDoubleDS;
    ds.write = writeDoubleDS;

    //�������� DS � ����. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // ������� �������� � ������ ������ �������� ������� ��������� ��� �������� ����

        return ret;
    }

    return UA_STATUSCODE_GOOD;
}

// ������� ��� ���������� ������������ ������
static UA_StatusCode
addReferenceMandatory(UA_Server* server, UA_NodeId nodeId) {
    return UA_Server_addReference(server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
        true);
}

static UA_NodeId
addPIDControllerType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PIDControllerType");
    UA_NodeId pidControllerTypeId;
    UA_Server_addObjectTypeNode(server, UA_NODEID_STRING(1, "PIDControllerType"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "PIDControllerType"),
        varAttr, NULL, &pidControllerTypeId);

    UA_VariableAttributes nmAttr = UA_VariableAttributes_default;
    nmAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PIDName");
    UA_NodeId pidNameId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "PIDName"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        nmAttr, NULL, &pidNameId);
    addReferenceMandatory(server, pidNameId);



    UA_VariableAttributes kpAttr = UA_VariableAttributes_default;
    kpAttr.displayName = UA_LOCALIZEDTEXT("en-US", "KP");
    kpAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    kpAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId kpId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "KP"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        kpAttr, NULL, &kpId);
    addReferenceMandatory(server, kpId);



    UA_VariableAttributes kiAttr = UA_VariableAttributes_default;
    kiAttr.displayName = UA_LOCALIZEDTEXT("en-US", "KI");
    kiAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    kiAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId kiId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "KI"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        kiAttr, NULL, &kiId);
    addReferenceMandatory(server, kiId);


    UA_VariableAttributes kdAttr = UA_VariableAttributes_default;
    kdAttr.displayName = UA_LOCALIZEDTEXT("en-US", "KD");
    kdAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    kdAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId kdId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "KD"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        kdAttr, NULL, &kdId);
    addReferenceMandatory(server, kdId);


    UA_VariableAttributes outputAttr = UA_VariableAttributes_default;
    outputAttr.displayName = UA_LOCALIZEDTEXT("en-US", "OUTPUT");
    outputAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    outputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId outputId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OUTPUT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outputAttr, NULL, &outputId);
    addReferenceMandatory(server, outputId);


    UA_VariableAttributes setPointAttr = UA_VariableAttributes_default;
    setPointAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SETPOINT");
    setPointAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    setPointAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;


    UA_NodeId setPointId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SETPOINT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        setPointAttr, NULL, &setPointId);
    addReferenceMandatory(server, setPointId);


    UA_VariableAttributes processValueAttr = UA_VariableAttributes_default;
    processValueAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PROCESSVALUE");
    processValueAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    processValueAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;


    UA_NodeId processValueId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "PROCESSVALUE"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        processValueAttr, NULL, &processValueId);
    addReferenceMandatory(server, processValueId);


    UA_VariableAttributes modeAttr = UA_VariableAttributes_default;
    modeAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MODE");
    modeAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    modeAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;


    UA_NodeId modeId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "MODE"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        modeAttr, NULL, &modeId);
    addReferenceMandatory(server, modeId);

    return pidControllerTypeId;
}
