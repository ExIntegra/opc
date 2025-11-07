#include <stdio.h>
#include <math.h>
#include "opcuaSettings.h"
#include "types.h"

/* Функция для чтения данных из узлов (свойства) экземпляра регулятора. Работает
 * с компонентами экземпляра PID (kp, ki, kd, output, setPoint, processValue)*/
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

    /* читаем значение из nodeContext */
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
    if (server && nodeId) {
        UA_QualifiedName bn;
        UA_StatusCode rc = UA_Server_readBrowseName(server, *nodeId, &bn);
        if (rc == UA_STATUSCODE_GOOD) {
            printf("readDoubleDS: %.*s = %.3f\n",
                (int)bn.name.length, bn.name.data, v);
            UA_QualifiedName_clear(&bn);
        }
        else {
            /* fallback: печатаем NodeId, если browseName не прочитали */
            if (nodeId->identifierType == UA_NODEIDTYPE_NUMERIC) {
                printf("readDoubleDS: ns=%u;i=%u = %.3f\n",
                    nodeId->namespaceIndex,
                    nodeId->identifier.numeric,
                    v);
            }
            else {
                printf("readDoubleDS: <unknown node> = %.3f\n", v);
            }
        }
    }
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
static UA_StatusCode find_child_var(UA_Server* server,
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
static UA_StatusCode attach_child_double(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);

    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

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

static UA_StatusCode attach_child_read_only_double(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {
    UA_NodeId childId = UA_NODEID_NULL;
    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readDoubleDS;
    ds.write = NULL; // Только чтение
    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла
        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode attach_child_bool(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

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

static UA_StatusCode attach_child_only_read_bool(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {
    UA_NodeId childId = UA_NODEID_NULL;
    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readBoolDS;
    ds.write = NULL; // Только чтение
    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла
        return ret;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode attach_child_UInt32(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {

    UA_NodeId childId = UA_NODEID_NULL;

    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }

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

UA_StatusCode attach_child_only_read_UInt32(UA_Server* server,
    const UA_NodeId parent,
    const char* browseName,
    void* ptrToField) {
    UA_NodeId childId = UA_NODEID_NULL;
    //находим узел и передаем его в chilId через указатель:
    UA_StatusCode ret = find_child_var(server, parent, browseName, &childId);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    ret = UA_Server_setNodeContext(server, childId, ptrToField);
    if (ret != UA_STATUSCODE_GOOD) {
        return ret;
    }
    // Устанавливаем DataSource.
    UA_DataSource ds;
    ds.read = readUInt32DS;
    ds.write = NULL; // Только чтение
    //Привязка DS к узлу. 
    ret = UA_Server_setVariableNode_dataSource(server, childId, ds);
    if (ret != UA_STATUSCODE_GOOD) {
        UA_Server_setNodeContext(server, childId, NULL); // Убираем контекст в случае ошибки привязки очистки контекста при удаление узла
        return ret;
    }
    return UA_STATUSCODE_GOOD;
}
    
    // Функция для добавления обязательных ссылок
static UA_StatusCode add_reference_mandatory(UA_Server* server, UA_NodeId nodeId) {
    return UA_Server_addReference(server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
        true);
}

// Глобальная переменная для хранения NodeId
UA_NodeId pidControllerTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1001 } };
UA_NodeId sensorTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1002 } };
UA_NodeId valveTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1003 } };
UA_NodeId reactorTypeId = { 1, UA_NODEIDTYPE_NUMERIC, { 1004 } };
UA_NodeId valveHandleControlType = { 1, UA_NODEIDTYPE_NUMERIC, { 1005 } };

UA_NodeId addValveHandleControlType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ValveHandleControlType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1005),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "ValveHandleControlType"),
        varAttr, NULL, &valveHandleControlType);

	UA_VariableAttributes nameAttr = UA_VariableAttributes_default;
	nameAttr.displayName = UA_LOCALIZEDTEXT("en-US", "NAME");
	nameAttr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
	nameAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId nameId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, valveHandleControlType,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "NAME"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		nameAttr, NULL, &nameId);
	add_reference_mandatory(server, nameId);

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

UA_NodeId addReactorType(UA_Server* server) {
    UA_ObjectTypeAttributes varAttr = UA_ObjectTypeAttributes_default;
    varAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ReactorType");
    UA_Server_addObjectTypeNode(server,
        UA_NODEID_NUMERIC(1, 1004),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "ReactorType"),
        varAttr, NULL, &reactorTypeId);

    UA_VariableAttributes nameAttr = UA_VariableAttributes_default;
    nameAttr.displayName = UA_LOCALIZEDTEXT("en-US", "NAME");
    nameAttr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    nameAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId nameId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, reactorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "NAME"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        nameAttr, NULL, &nameId);
    add_reference_mandatory(server, nameId);

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
    add_reference_mandatory(server, nameId);

	UA_VariableAttributes actualAttr = UA_VariableAttributes_default;
	actualAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ACTUAL");
	actualAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
	actualAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId actualId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "ACTUAL"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		actualAttr, NULL, &actualId);
    add_reference_mandatory(server, actualId);

    UA_VariableAttributes clampEnableAttr = UA_VariableAttributes_default;
    clampEnableAttr.displayName = UA_LOCALIZEDTEXT("en-US", "CLAMP_ENABLE");
    clampEnableAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    clampEnableAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId clampEnableId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "CLAMP_ENABLE"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        clampEnableAttr, NULL, &clampEnableId);
    add_reference_mandatory(server, clampEnableId);

    UA_VariableAttributes outMaxAttr = UA_VariableAttributes_default;
    outMaxAttr.displayName = UA_LOCALIZEDTEXT("en-US", "OUTPUT_MAX");
    outMaxAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    outMaxAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId outMaxId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OUTPUT_MAX"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outMaxAttr, NULL, &outMaxId);
    add_reference_mandatory(server, outMaxId);

    UA_VariableAttributes outMinAttr = UA_VariableAttributes_default;
    outMinAttr.displayName = UA_LOCALIZEDTEXT("en-US", "OUTPUT_MIN");
    outMinAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    outMinAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId outMinId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "OUTPUT_MIN"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        outMinAttr, NULL, &outMinId);
    add_reference_mandatory(server, outMinId);

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
    add_reference_mandatory(server, actionHHId);

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
    add_reference_mandatory(server, actionLLId);

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
    add_reference_mandatory(server, safeOutputHHId);

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
    add_reference_mandatory(server, safeOutputLLId);

    UA_VariableAttributes commandAttr = UA_VariableAttributes_default;
    commandAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Command");
    commandAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    commandAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId commandId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, valveTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "Command"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        commandAttr, NULL, &commandId);
    add_reference_mandatory(server, commandId);
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

    UA_VariableAttributes lowAttr = UA_VariableAttributes_default;
    lowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SET_LOW");
    lowAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    lowAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId lowId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SET_LOW"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        lowAttr, NULL, &lowId);
    add_reference_mandatory(server, lowId);

    UA_VariableAttributes highAttr = UA_VariableAttributes_default;
    highAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SET_HIGH");
    highAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    highAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId highId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SET_HIGH"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        highAttr, NULL, &highId);
    add_reference_mandatory(server, highId);

    UA_VariableAttributes lowLowAttr = UA_VariableAttributes_default;
    lowLowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SET_LOW_LOW");
    lowLowAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    lowLowAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId lowLowId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SET_LOW_LOW"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        lowLowAttr, NULL, &lowLowId);
    add_reference_mandatory(server, lowLowId);

    UA_VariableAttributes highHighAttr = UA_VariableAttributes_default;
    highHighAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SET_HIGH_HIGH");
    highHighAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    highHighAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId highHighId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SET_HIGH_HIGH"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        highHighAttr, NULL, &highHighId);
    add_reference_mandatory(server, highHighId);

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
    add_reference_mandatory(server, hysteresisId);

	UA_VariableAttributes alarmLowAttr = UA_VariableAttributes_default;
	alarmLowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ALARM_LOW");
	alarmLowAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
	alarmLowAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId alarmLowId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "ALARM_LOW"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		alarmLowAttr, NULL, &alarmLowId);
    add_reference_mandatory(server, alarmLowId);

	UA_VariableAttributes alarmHighAttr = UA_VariableAttributes_default;
	alarmHighAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ALARM_HIGH");
	alarmHighAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
	alarmHighAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId alarmHighId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "ALARM_HIGH"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		alarmHighAttr, NULL, &alarmHighId);
    add_reference_mandatory(server, alarmHighId);

	UA_VariableAttributes alarmLowLowAttr = UA_VariableAttributes_default;
	alarmLowLowAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ALARM_LOW_LOW");
	alarmLowLowAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
	alarmLowLowAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId alarmLowLowId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "ALARM_LOW_LOW"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		alarmLowLowAttr, NULL, &alarmLowLowId);
    add_reference_mandatory(server, alarmLowLowId);

	UA_VariableAttributes alarmHighHighAttr = UA_VariableAttributes_default;
	alarmHighHighAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ALARM_HIGH_HIGH");
	alarmHighHighAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
	alarmHighHighAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId alarmHighHighId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "ALARM_HIGH_HIGH"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		alarmHighHighAttr, NULL, &alarmHighHighId);
    add_reference_mandatory(server, alarmHighHighId);

	UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
	statusAttr.displayName = UA_LOCALIZEDTEXT("en-US", "STATUS");
	statusAttr.dataType = UA_TYPES[UA_TYPES_UINT32].typeId;
	statusAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
	UA_NodeId statusId;
	UA_Server_addVariableNode(server, UA_NODEID_NULL, sensorTypeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "STATUS"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		statusAttr, NULL, &statusId);
    add_reference_mandatory(server, statusId);
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
    add_reference_mandatory(server, pidNameId);

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
    add_reference_mandatory(server, kpId);

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
    add_reference_mandatory(server, kiId);

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
    add_reference_mandatory(server, kdId);

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
    add_reference_mandatory(server, outputId);

    UA_VariableAttributes manualOutputAttr = UA_VariableAttributes_default;
    manualOutputAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MANUAL_OUTPUT");
    manualOutputAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    manualOutputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId manualOutputValueId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "MANUAL_OUTPUT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        manualOutputAttr, NULL, &manualOutputValueId);
    add_reference_mandatory(server, manualOutputValueId);

    UA_VariableAttributes setPointAttr = UA_VariableAttributes_default;
    setPointAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SET_POINT");
    setPointAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    setPointAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId setPointId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SET_POINT"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        setPointAttr, NULL, &setPointId);
    add_reference_mandatory(server, setPointId);

    UA_VariableAttributes processValueAttr = UA_VariableAttributes_default;
    processValueAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PROCESS_VALUE");
    processValueAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    processValueAttr.accessLevel = UA_ACCESSLEVELMASK_READ;
    UA_NodeId processValueId;
    UA_Server_addVariableNode(server, UA_NODEID_NULL, pidControllerTypeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "PROCESS_VALUE"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        processValueAttr, NULL, &processValueId);
    add_reference_mandatory(server, processValueId);

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
    add_reference_mandatory(server, modeId);
    return pidControllerTypeId;
}

UA_StatusCode opc_ua_create_valve_handle_control(UA_Server* server,
    UA_NodeId parentFolder, const char* valveHandleControlName, ValveHandleControl* valveHandleControl) {
    UA_NodeId valveHandleControlObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация ID
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),       // Тип связи (Optional)
        UA_QUALIFIEDNAME(1, (char*)valveHandleControlName),    // Имя клапана (например Valve1)
        valveHandleControlType,      // Тип объекта ValveHandleControlType
        UA_ObjectAttributes_default, NULL, &valveHandleControlObjId);  // Атрибуты по умолчанию
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object valve handle control %s\n", valveHandleControlName);
        return rc;
    }
    else {
        printf("Valve Handle Control %s created successfully\n", valveHandleControlName);
    }
    valveHandleControl->objId = valveHandleControlObjId;
    // Привязываем переменные ValveHandleControl из объекта к полям структуры ValveHandleControl
    rc = attach_child_double(server, valveHandleControlObjId, "MANUAL_OUTPUT", &valveHandleControl->manualoutput); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

/* --- Функция для создания объекта PID из типа PIDControllerType --- */
UA_StatusCode opc_ua_create_pid_instance(UA_Server* server, UA_NodeId parentFolder ,const char* pidName, PIDControllerType* pid) {
    UA_NodeId pidObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация IПD
        parentFolder,
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
    rc = attach_child_double(server, pidObjId, "KP", &pid->kp); if (rc) return rc;
    rc = attach_child_double(server, pidObjId, "KI", &pid->ki); if (rc) return rc;
    rc = attach_child_double(server, pidObjId, "KD", &pid->kd); if (rc) return rc;
    rc = attach_child_double(server, pidObjId, "SET_POINT", &pid->setpoint); if (rc) return rc;
    rc = attach_child_double(server, pidObjId, "PROCESS_VALUE", &pid->processvalue); if (rc) return rc;
    rc = attach_child_read_only_double(server, pidObjId, "OUTPUT", &pid->output); if (rc) return rc;
    rc = attach_child_double(server, pidObjId, "MANUAL_OUTPUT", &pid->manualoutput); if (rc) return rc;
    rc = attach_child_bool(server, pidObjId, "MODE", &pid->mode); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode opc_ua_create_reactor_instance(UA_Server* server,
    UA_NodeId parentFolder ,const char* reactorName, Reactor* reactor) {
    UA_NodeId reactorObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация ID
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),       // Тип связи (Optional)
        UA_QUALIFIEDNAME(1, (char*)reactorName),    // Имя реактора (например Reactor1)
        reactorTypeId,      // Тип объекта ReactorType
        UA_ObjectAttributes_default, NULL, &reactorObjId);  // Атрибуты по умолчанию
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object reactor %s\n", reactorName);
        return rc;
    }
    else {
        printf("Reactor %s created successfully\n", reactorName);
    }
    reactor->objId = reactorObjId;
    // Привязываем переменные Reactor из объекта к полям структуры Reactor
    rc = attach_child_double(server, reactorObjId, "REACTOR_VOLUME", &reactor->volume); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode opc_ua_create_sensor_instance(UA_Server* server,
    UA_NodeId parentFolder, const char* sensorName,
    UA_Boolean enableAlarms,
    Sensor* sensor)
{
    /* 1) Создаём объект SENSOR из вашего SensorType */
    UA_NodeId sensorObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,
        parentFolder,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, (char*)sensorName),
        sensorTypeId, /* ваш тип */
        UA_ObjectAttributes_default, NULL, &sensorObjId);
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Failed to add object sensor %s\n", sensorName);
        return rc;
    }
    else {
        printf("Sensor %s created successfully\n", sensorName);
    }

    sensor->objId = sensorObjId;
    sensor->alarmConditionId = UA_NODEID_NULL;
    sensor->alarmsEnabled = enableAlarms;

    /* 2) Привязываем переменные SENSOR к полям структуры */
    rc = attach_child_double(server, sensorObjId, "PROCESS_VALUE", &sensor->io.pv);            if (rc) return rc;
    rc = attach_child_double(server, sensorObjId, "SET_LOW", &sensor->limits.low);             if (rc) return rc;
    rc = attach_child_double(server, sensorObjId, "SET_HIGH", &sensor->limits.high);           if (rc) return rc;
    rc = attach_child_double(server, sensorObjId, "SET_LOW_LOW", &sensor->limits.lowLow);      if (rc) return rc;
    rc = attach_child_double(server, sensorObjId, "SET_HIGH_HIGH", &sensor->limits.highHigh);  if (rc) return rc;
    rc = attach_child_UInt32(server, sensorObjId, "STATUS", &sensor->io.st);                   if (rc) return rc;
    rc = attach_child_double(server, sensorObjId, "HYSTERESIS", &sensor->limits.hysteresis);   if (rc) return rc;
    rc = attach_child_bool(server, sensorObjId, "ALARM_LOW", &sensor->state.low);              if (rc) return rc;
    rc = attach_child_bool(server, sensorObjId, "ALARM_HIGH", &sensor->state.high);            if (rc) return rc;
    rc = attach_child_bool(server, sensorObjId, "ALARM_LOW_LOW", &sensor->state.lowLow);       if (rc) return rc;
    rc = attach_child_bool(server, sensorObjId, "ALARM_HIGH_HIGH", &sensor->state.highHigh);   if (rc) return rc;

    /* 3) Если тревоги не нужны — выходим */
    if (!enableAlarms) {
        /* на всякий случай глушим события у узла сенсора */
        UA_Byte ev = 0;
        (void)UA_Server_writeEventNotifier(server, sensorObjId, ev);
        sensor->alarmConditionId = UA_NODEID_NULL;
        return UA_STATUSCODE_GOOD;
    }

    /* 4) Создаём Condition NonExclusiveLimitAlarmType как дочерний узел сенсора */
    UA_StatusCode rcA = UA_Server_createCondition(
        server,
        UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_NONEXCLUSIVELIMITALARMTYPE),
        UA_QUALIFIEDNAME(1, "PV_LimitAlarm"),
        sensorObjId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        &sensor->alarmConditionId);

    if (rcA != UA_STATUSCODE_GOOD) {
        printf("CreateCondition failed: %s\n", UA_StatusCode_name(rcA));
        sensor->alarmConditionId = UA_NODEID_NULL; /* продолжаем без тревог */
        return UA_STATUSCODE_GOOD;
    }

    /* 5) Включаем Condition: EnabledState/Id = true */
    {
        UA_Boolean en = UA_TRUE;
        UA_Variant v; UA_Variant_setScalar(&v, &en, &UA_TYPES[UA_TYPES_BOOLEAN]);
        (void)UA_Server_setConditionVariableFieldProperty(
            server, sensor->alarmConditionId, &v,
            UA_QUALIFIEDNAME(0, "EnabledState"),
            UA_QUALIFIEDNAME(0, "Id"));
    }

    /* 6) Retain = true и SourceNode = SENSOR */
    {
        UA_Boolean retain = UA_TRUE;
        UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "Retain"), &retain, &UA_TYPES[UA_TYPES_BOOLEAN]);

        UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
            UA_QUALIFIEDNAME(0, "SourceNode"), &sensorObjId, &UA_TYPES[UA_TYPES_NODEID]);
    }

    /* 7) Инициализируем лимиты для LimitAlarm */
    UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
        UA_QUALIFIEDNAME(0, "HighLimit"), &sensor->limits.high, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
        UA_QUALIFIEDNAME(0, "HighHighLimit"), &sensor->limits.highHigh, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
        UA_QUALIFIEDNAME(0, "LowLimit"), &sensor->limits.low, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_writeObjectProperty_scalar(server, sensor->alarmConditionId,
        UA_QUALIFIEDNAME(0, "LowLowLimit"), &sensor->limits.lowLow, &UA_TYPES[UA_TYPES_DOUBLE]);

    return UA_STATUSCODE_GOOD;
}


UA_StatusCode opc_ua_create_valve_instance(UA_Server* server,
    UA_NodeId parentFolder ,const char* valveName, Valve* valve) {
    UA_NodeId valveObjId;
    UA_StatusCode rc = UA_Server_addObjectNode(server,
        UA_NODEID_NULL,           // Генерация ID
        parentFolder,
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
    rc = attach_child_bool(server, valveObjId, "CLAMP_ENABLE", &valve->clampEnable); if (rc) return rc;
    rc = attach_child_double(server, valveObjId, "OUTPUT_MAX", &valve->outMax); if (rc) return rc;
    rc = attach_child_double(server, valveObjId, "OUTPUT_MIN", &valve->outMin); if (rc) return rc;
    rc = attach_child_UInt32(server, valveObjId, "actionHH", &valve->actionHH); if (rc) return rc;
    rc = attach_child_UInt32(server, valveObjId, "actionLL", &valve->actionLL); if (rc) return rc;
    rc = attach_child_double(server, valveObjId, "SafeOutputHH", &valve->safeOutputHH); if (rc) return rc;
    rc = attach_child_double(server, valveObjId, "SafeOutputLL", &valve->safeOutputLL); if (rc) return rc;
    rc = attach_child_read_only_double(server, valveObjId, "Command", &valve->command); if (rc) return rc;
	rc = attach_child_read_only_double(server, valveObjId, "ACTUAL", &valve->actual_position); if (rc) return rc;
    return UA_STATUSCODE_GOOD;
}

// --- Функция для создания папки (folder) для ячейки (cell) в адресном пространстве OPC UA --- //
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