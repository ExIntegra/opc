#include <stdio.h>
#include <math.h>
#include "opcuaSettings.h"
#include "DAQ.h"

static UA_StatusCode readDoubleDS(UA_Server* server,
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

    /* Приводим nodeContext к типу double и разыменовываем указатель */
    const UA_Double v = *(const UA_Double*)nodeContext;

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


/* Функция для записи данных в узлы (свойства) экземпляра регулятора. Работает
 * с компонентами экземпляра PID (kp, ki, kd, output, setPoint, processValue)*/
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

    //Проверка на валидность данных
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

    /* Записываем по адресу, переданному через nodeContext */
    *(UA_Double*)nodeContext = v;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeBoolDS(UA_Server* server,
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

    if (data->value.type != &UA_TYPES[UA_TYPES_BOOLEAN] ||
        data->value.data == NULL ||
        data->value.arrayLength != 0 ||
        data->value.arrayDimensionsSize != 0)
        return UA_STATUSCODE_BADTYPEMISMATCH;

    const UA_Boolean v = *(const UA_Boolean*)data->value.data;
    /* Для булева ничего не «проверяем на конечность» */
    *(UA_Boolean*)nodeContext = v;
    return UA_STATUSCODE_GOOD;
}


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

    /* Приводим nodeContext к типу UA_UInt32 и разыменовываем указатель */
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

static UA_StatusCode readBoolDS(UA_Server* server,
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

    /* Приодим nodeContext к типу UA_Boolean и разыменовываем указатель */
    const UA_Boolean v = *(const UA_Boolean*)nodeContext;

    UA_StatusCode rv = UA_Variant_setScalarCopy(&out->value, &v, &UA_TYPES[UA_TYPES_BOOLEAN]);
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

/* Функия для нахождения компонента (cвойства) родителя (экземпляра регулятора) и передача его в nodeId
 * для дальнейше работы с ним.
 * parent - NodeId родителя (объект)
 * browseName - имя искомого компонента родителя
 * out - nodeId найденного комнонента родителя*/
static UA_StatusCode findChildVar(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    UA_NodeId* out) {

    //Описываем шаг, по которому пойдет поиск:
    UA_RelativePathElement rpe; //создаем экземпляр структуры
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT); //тип связи родителя и компненнта
    //Идем вперед от родителя к компоненту.
    rpe.isInverse = false;
    rpe.includeSubtypes = false; //не учитываем подтипы ссылки
    //Имя цели, к которому идем в поиске.
    rpe.targetName = UA_QUALIFIEDNAME(1, (char*)browseName);
    //Создали шаг и переходим к созаднию маршрута из BrowsePath.

    //Собираем маршрут из  экземпляра структуры BrowsePath , стартуя от parent, передаем маршрут rpe.
    UA_BrowsePath bp; //экземпляр стурктуры Browsepath
    UA_BrowsePath_init(&bp);
    bp.startingNode = parent; //откуда начинаем поиск
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;

    //Производим поиск, а UA_BrowsePathResult brp является экземпляром структуры результата поиска.
    //имеет statrusCode - статус самой операции и targetSize - сколько целей нашли.
    UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(server, &bp);

    //Проверяем результат поска.
    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1) {
        UA_BrowsePathResult_clear(&bpr);
        return UA_STATUSCODE_BADNOTFOUND;
    }

    /* Берём первый найденный таргет и копируем локальный NodeId */
    UA_StatusCode c = UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, out);

    /* Чистим временные аллокации результата */
    UA_BrowsePathResult_clear(&bpr);
    ///???
    return (c == UA_STATUSCODE_GOOD) ? UA_STATUSCODE_GOOD : c;

}

/*Функция для связывания nodeContext с найденным узлом
 *с помощью функции findChildVar.
 *parent - родительский узел
 *name - имя искомого узла
 *pid - экземпляр структуры pid
 *field - компонент экземпляра к которому привязывается nodeContext (KP, KI, ...)*/
static UA_StatusCode attachChild(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = findChildVar(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);

    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readDoubleDS;
    ds.write = writeDoubleDS;

    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла
        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode attachChildBool(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = findChildVar(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);

    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readBoolDS;
    ds.write = writeBoolDS;

    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла

        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode attachChildUInt32(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = findChildVar(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);

    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readUInt32DS;
    ds.write = writeUInt32DS;

    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла

        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

// Функция для добавления обязательных ссылок
static UA_StatusCode addReferenceMandatory(UA_Server* server, UA_NodeId nodeId) {
    return UA_Server_addReference(server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
        true);
}

// Глобальная переменная для хранения NodeId типа PIDControllerType
UA_NodeId pidControllerTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1001 } };
UA_NodeId sensorTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1002 } };
UA_NodeId valveTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1003 } };

// Функция для создания типа ValveType
UA_NodeId addValveType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ValveType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1003),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "ValveType"),
        varAttr, NULL, &valveTypeId);

    UA_VariableAttributes nameAttr = UA_VariableAttributes_default;
    nameAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Name");
    nameAttr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    nameAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId nameId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "Name"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        nameAttr, NULL, &nameId);
    addReferenceMandatory(server, nameId);

    UA_VariableAttributes clampEnableAttr = UA_VariableAttributes_default;
    clampEnableAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ClampEnable");
    clampEnableAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    clampEnableAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId clampEnableId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "ClampEnable"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        clampEnableAttr, NULL, &clampEnableId);
    addReferenceMandatory(server, clampEnableId);

    UA_VariableAttributes outMaxAttr = UA_VariableAttributes_default;
    outMaxAttr.displayName = UA_LOCALIZEDTEXT("en-US", "OutMax");
    outMaxAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    outMaxAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId outMaxId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OutMax"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outMaxAttr, NULL, &outMaxId);
    addReferenceMandatory(server, outMaxId);

    UA_VariableAttributes outMinAttr = UA_VariableAttributes_default;
    outMinAttr.displayName = UA_LOCALIZEDTEXT("en-US", "OutMin");
    outMinAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    outMinAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId outMinId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OutMin"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outMinAttr, NULL, &outMinId);
    addReferenceMandatory(server, outMinId);

    UA_VariableAttributes actionHHAttr = UA_VariableAttributes_default;
    actionHHAttr.displayName = UA_LOCALIZEDTEXT("en-US", "actionHH");
    actionHHAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
    actionHHAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId actionHHId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "actionHH"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        actionHHAttr, NULL, &actionHHId);
    addReferenceMandatory(server, actionHHId);

    UA_VariableAttributes actionLLAttr = UA_VariableAttributes_default;
    actionLLAttr.displayName = UA_LOCALIZEDTEXT("en-US", "actionLL");
    actionLLAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
    actionLLAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId actionLLId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "actionLL"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        actionLLAttr, NULL, &actionLLId);
    addReferenceMandatory(server, actionLLId);

    UA_VariableAttributes safeOutputHHAttr = UA_VariableAttributes_default;
    safeOutputHHAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SafeOutputHH");
    safeOutputHHAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    safeOutputHHAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId safeOutputHHId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SafeOutputHH"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        safeOutputHHAttr, NULL, &safeOutputHHId);
    addReferenceMandatory(server, safeOutputHHId);

    UA_VariableAttributes safeOutputLLAttr = UA_VariableAttributes_default;
    safeOutputLLAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SafeOutputLL");
    safeOutputLLAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    safeOutputLLAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId safeOutputLLId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SafeOutputLL"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        safeOutputLLAttr, NULL, &safeOutputLLId);
    addReferenceMandatory(server, safeOutputLLId);

    UA_VariableAttributes commandAttr = UA_VariableAttributes_default;
    commandAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Command");
    commandAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    commandAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId commandId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "Command"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        commandAttr, NULL, &commandId);
    addReferenceMandatory(server, commandId);

    UA_VariableAttributes actualAttr = UA_VariableAttributes_default;
    actualAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Actual");
    actualAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    actualAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId actualId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "Actual"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        actualAttr, NULL, &actualId);
    addReferenceMandatory(server, actualId);
    return valveTypeId;
}

// Функция для создания типа SensorType
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
    pvAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PV");
    pvAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    pvAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId pvId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "PV"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        pvAttr, NULL, &pvId);
    addReferenceMandatory(server, pvId);

    UA_VariableAttributes stAttr = UA_VariableAttributes_default;
    stAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ST");
    stAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
    stAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId stId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "ST"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        stAttr, NULL, &stId);
    addReferenceMandatory(server, stId);

    UA_VariableAttributes lowAttr = UA_VariableAttributes_default;
    lowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "LOWLIMIT");
    lowAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    lowAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId lowId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "LOWLIMIT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        lowAttr, NULL, &lowId);
    addReferenceMandatory(server, lowId);

    UA_VariableAttributes highAttr = UA_VariableAttributes_default;
    highAttr.displayName = UA_LOCALIZEDTEXT("en-US", "HIGHLIMIT");
    highAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    highAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId highId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "HIGHLIMIT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        highAttr, NULL, &highId);
    addReferenceMandatory(server, highId);

    UA_VariableAttributes lowLowAttr = UA_VariableAttributes_default;
    lowLowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "LOWLOWLIMIT");
    lowLowAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    lowLowAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId lowLowId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "LOWLOWLIMIT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        lowLowAttr, NULL, &lowLowId);
    addReferenceMandatory(server, lowLowId);

    UA_VariableAttributes highHighAttr = UA_VariableAttributes_default;
    highHighAttr.displayName = UA_LOCALIZEDTEXT("en-US", "HIGHHIGHLIMIT");
    highHighAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    highHighAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId highHighId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "HIGHHIGHLIMIT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        highHighAttr, NULL, &highHighId);
    addReferenceMandatory(server, highHighId);

    UA_VariableAttributes hysteresisAttr = UA_VariableAttributes_default;
    hysteresisAttr.displayName = UA_LOCALIZEDTEXT("en-US", "HYSTERESIS");
    hysteresisAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    hysteresisAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId hysteresisId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "HYSTERESIS"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        hysteresisAttr, NULL, &hysteresisId);
    addReferenceMandatory(server, hysteresisId);
    return sensorTypeId;
}

// Функция для создания типа PIDControllerType
UA_NodeId addPIDControllerType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PIDControllerType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1001),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "PIDControllerType"),
        varAttr, NULL, &pidControllerTypeId);

    UA_VariableAttributes nmAttr = UA_VariableAttributes_default;
    nmAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PIDName");
    nmAttr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    nmAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
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
    outputAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId outputId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OUTPUT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outputAttr, NULL, &outputId);
    addReferenceMandatory(server, outputId);

    UA_VariableAttributes manualOutputAttr = UA_VariableAttributes_default;
    manualOutputAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MANUALOUTPUT");
    manualOutputAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    manualOutputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId manualOutputValueId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "MANUALOUTPUT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        manualOutputAttr, NULL, &manualOutputValueId);
    addReferenceMandatory(server, manualOutputValueId);

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
    processValueAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
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


/* --- Функция для создания объекта PID из типа PIDControllerType --- */
UA_StatusCode opcua_create_pid_instance(UA_Server* server, const char* pidName, ControlLoop* loop) {
    UA_NodeId pidObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация IПD
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),   // Родитель (ObjectsFolder)
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),       // Тип связи (Optional)
        UA_QUALIFIEDNAME(1, (char*)pidName),            // Имя PID (например PID1)
        pidControllerTypeId,                            // Тип объекта PIDControllerType
        UA_ObjectAttributes_default, NULL, &pidObjId);  // Атрибуты по умолчанию

    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object PID %s\n", pidName);
        return rc;
    }
    else {
		printf("PID %s created successfully\n", pidName);
    }
    /* 2) Привязываем переменные PID из объекта к полям структуры ControlLoop */
    rc = attachChild(server, pidObjId, "KP", &loop->pid.kp); if (rc) return rc;
    rc = attachChild(server, pidObjId, "KI", &loop->pid.ki); if (rc) return rc;
    rc = attachChild(server, pidObjId, "KD", &loop->pid.kd); if (rc) return rc;
    rc = attachChild(server, pidObjId, "SETPOINT", &loop->pid.setpoint); if (rc) return rc;
    rc = attachChild(server, pidObjId, "PROCESSVALUE", &loop->pid.processvalue); if (rc) return rc;
    rc = attachChild(server, pidObjId, "OUTPUT", &loop->pid.output); if (rc) return rc;
    rc = attachChild(server, pidObjId, "MANUALOUTPUT", &loop->pid.manualoutput); if (rc) return rc;
    rc = attachChildBool(server, pidObjId, "MODE", &loop->pid.mode); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode opcua_create_sensor_instance(UA_Server* server, const char* sensorName, Sensor* sensor) {
    UA_NodeId sensorObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация ID
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),   // Родитель (ObjectsFolder)
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),       // Тип связи (Optional)
        UA_QUALIFIEDNAME(1, (char*)sensorName),    // Имя сенсора (например Sensor1)
        sensorTypeId,      // Тип объекта SensorType
        UA_ObjectAttributes_default, NULL, &sensorObjId);  // Атрибуты по умолчанию

    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object sensor %s\n", sensorName);
        return rc;
    }

    else {
		printf("Sensor %s created successfully\n", sensorName);
    }
	sensor->objId = sensorObjId;
    /* 2) Привязываем переменные Sensor из объекта к полям структуры CashSensor */
    rc = attachChild(server, sensorObjId, "PV", &sensor->io.pv); if (rc) return rc;
    rc = attachChildUInt32(server, sensorObjId, "ST", &sensor->io.st); if (rc) return rc;
    rc = attachChild(server, sensorObjId, "LOWLIMIT", &sensor->limits.low); if (rc) return rc;
    rc = attachChild(server, sensorObjId, "HIGHLIMIT", &sensor->limits.high); if (rc) return rc;
    rc = attachChild(server, sensorObjId, "LOWLOWLIMIT", &sensor->limits.lowLow); if (rc) return rc;
    rc = attachChild(server, sensorObjId, "HIGHHIGHLIMIT", &sensor->limits.highHigh); if (rc) return rc;
	rc = attachChild(server, sensorObjId, "HYSTERESIS", &sensor->limits.hysteresis); if (rc) return rc;

    /* === NonExclusiveLimitAlarmType на экземпляре датчика === */
    sensor->alarmConditionId = UA_NODEID_NULL;
    UA_StatusCode rcA = UA_Server_createCondition(
        server,
        UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_NONEXCLUSIVELIMITALARMTYPE),
        UA_QUALIFIEDNAME(1, "LimitAlarm"),
        sensorObjId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        &sensor->alarmConditionId);

    if (rcA == UA_STATUSCODE_GOOD) {
        UA_Boolean en = UA_TRUE, ret = UA_TRUE;
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "EnabledState/Id"), &en, &UA_TYPES[UA_TYPES_BOOLEAN]);
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "Retain"), &ret, &UA_TYPES[UA_TYPES_BOOLEAN]);

        /* стартовые пределы */
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "HighLimit"), &sensor->limits.high, &UA_TYPES[UA_TYPES_DOUBLE]);
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "HighHighLimit"), &sensor->limits.highHigh, &UA_TYPES[UA_TYPES_DOUBLE]);
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "LowLimit"), &sensor->limits.low, &UA_TYPES[UA_TYPES_DOUBLE]);
        (void)UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "LowLowLimit"), &sensor->limits.lowLow, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode opcua_create_valve_instance(UA_Server* server, const char* valveName, Valve* valve) {
    UA_NodeId valveObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация ID
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),   // Родитель (ObjectsFolder)
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),       // Тип связи (Optional)
        UA_QUALIFIEDNAME(1, (char*)valveName),    // Имя клапана (например Valve1)
        valveTypeId,      // Тип объекта ValveType
        UA_ObjectAttributes_default, NULL, &valveObjId);  // Атрибуты по умолчанию

    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object Valve %s\n", valveName);
        return rc;
    }
    else {
		printf("Valve %s created successfully\n", valveName);
		valve->objId = valveObjId;
    }
    /* Привязываем переменные Valve из объекта к полям структуры CashValve */
    rc = attachChildBool(server, valveObjId, "ClampEnable", &valve->clampEnable); if (rc) return rc;
    rc = attachChild(server, valveObjId, "OutMax", &valve->outMax); if (rc) return rc;
    rc = attachChild(server, valveObjId, "OutMin", &valve->outMin); if (rc) return rc;
    rc = attachChildUInt32(server, valveObjId, "actionHH", &valve->actionHH); if (rc) return rc;
    rc = attachChildUInt32(server, valveObjId, "actionLL", &valve->actionLL); if (rc) return rc;
    rc = attachChild(server, valveObjId, "SafeOutputHH", &valve->safeOutputHH); if (rc) return rc;
    rc = attachChild(server, valveObjId, "SafeOutputLL", &valve->safeOutputLL); if (rc) return rc;
    rc = attachChild(server, valveObjId, "Command", &valve->command); if (rc) return rc;
	rc = attachChild(server, valveObjId, "Actual", &valve->actual); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}