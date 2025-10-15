#ifndef SERVICE_NOW_C
#define SERVICE_NOW_C

// char* sn_data_autocomplete = "{\"operationName\":\"snRecordReferenceConnected\",\"query\":\"query snRecordReferenceConnected($table:String!$field:String!$sys_id:String$encodedRecord:String$serializedChanges:String$chars:String!$ignoreRefQual:Glide_Boolean$paginationLimit:Int$paginationOffset:Int$sortBy:String$referenceKey:String$overrideReferenceTable:String$query:String$orderByDisplayColumn:Glide_Boolean){GlideLayout_Query{referenceDataRetriever(tableName:$table fieldName:$field encodedRecord:$encodedRecord serializedChanges:$serializedChanges pagination:{limit:$paginationLimit offset:$paginationOffset}ignoreTotalCount:true sysId:$sys_id chars:$chars sysparm_ignore_ref_qual:$ignoreRefQual sortBy:$sortBy sysparm_ref_override:$overrideReferenceTable query:$query orderByDisplayColumn:$orderByDisplayColumn referenceKey:$referenceKey){totalCount recentCount matchesCount referenceRecentDataList{sysId referenceKeyValue referenceData{key value}}referenceDataList{sysId referenceKeyValue referenceData{key value}}}}}\",\"variables\":{\"table\":\"interaction\",\"field\":\"opened_for\",\"sys_id\":\"-1\",\"encodedRecord\":\"77ee77ef77eSZjc4M2E5ZGUyYmYxNTIxMGZhMmJmY2U4NWU5MWJmNDjvt6zvt5Qx77es77etZUsyYmFzRzdZWjBUVEtCcGxrSW51UT09cjF4Xy1TX3JDR2NwWmV5M0wwYW55WHpEbnVmN0hKSWZGSHN4YWZaeUFvV2VnRDhrSzI3MnJDWm1rTzRqZFhTZl9NRzZya1hNQUhleTE1MHdQM01fODhVODVJd1ZqVHhHTzhvczluemVzWmdER1hkY3JxcDhEcHNTb2lKU0c2MjZSZVMtZjJzOWRJNlBYNzdhS3N0ZEJJb3JHWllHQUJSdlZuSThBY3RZeXdDZHlTZ19iNWc1U0wyQ1dfajVldWk0V0hkbm1uT3lBcFRJNmxGc3d6TEFOTGt4Q3JDOUVNTGFLb1p6RGtGNDdneG9nLWhmMzFkb1BOT3F2TE9PbTQwT3lGWW5IcjMzUmRyWERJOFJscEc3SFl6ejRUZUFMcUxTemhqVFZvUGVKRUxyNkUwV2ZMSnFvVG1pdGJIbWxIVWc5VUVaSF9WdldLZENrNjVTcVoyclpwVWhNcXl2eUJkZHlZTFVuZlpseXU5VEY0Wm5yc2RtUXBuMnFtcmFpQnBmNnNRZHItS05fSzItQlZyTjNuWG53aTliSkdrS0JJM3AySzYzRXhlb0V1dno1T09JNXlCWjhNWUs2TWVhTFkyVi1EVnQtdGp4SFRuSFZTTnRaUEI3bFoyTmRmc1Z4TVRteExpUVhNUElORTQ977eu77ev\",\"serializedChanges\":\"{\\\"assigned_to\\\":\\\"016ebac82b676210729af47ebe91bf2a\\\",\\\"assignment_group\\\":\\\"3efc509e87dd5910dd76873e8bbb354b\\\"}\",\"chars\":\"%s\",\"ignoreRefQual\":false,\"paginationLimit\":25,\"sortBy\":\"name\",\"referenceKey\":null,\"orderByDisplayColumn\":true},\"nowUxInteraction\":\"%s\",\"nowUiInteraction\":\"%s\",\"cacheable\":false,\"__unstableDisableBatching\":true,\"extensions\":{},\"queryContext\":null}";
char* sn_data_autocomplete = "{\"operationName\":\"snRecordReferenceConnected\",\"query\":\"query snRecordReferenceConnected($table:String!$field:String!$sys_id:String$encodedRecord:String$serializedChanges:String$chars:String!$ignoreRefQual:Glide_Boolean$paginationLimit:Int$paginationOffset:Int$sortBy:String$referenceKey:String$overrideReferenceTable:String$query:String$orderByDisplayColumn:Glide_Boolean){GlideLayout_Query{referenceDataRetriever(tableName:$table fieldName:$field encodedRecord:$encodedRecord serializedChanges:$serializedChanges pagination:{limit:$paginationLimit offset:$paginationOffset}ignoreTotalCount:true sysId:$sys_id chars:$chars sysparm_ignore_ref_qual:$ignoreRefQual sortBy:$sortBy sysparm_ref_override:$overrideReferenceTable query:$query orderByDisplayColumn:$orderByDisplayColumn referenceKey:$referenceKey){totalCount recentCount matchesCount referenceRecentDataList{sysId referenceKeyValue referenceData{key value}}referenceDataList{sysId referenceKeyValue referenceData{key value}}}}}\",\"variables\":{\"table\":\"interaction\",\"field\":\"opened_for\",\"sys_id\":\"-1\",\"encodedRecord\":\"77ee77ef77eSZjc4M2E5ZGUyYmYxNTIxMGZhMmJmY2U4NWU5MWJmNDjvt6zvt5Qx77es77etZUsyYmFzRzdZWjBUVEtCcGxrSW51UT09cjF4Xy1TX3JDR2NwWmV5M0wwYW55WHpEbnVmN0hKSWZGSHN4YWZaeUFvV2VnRDhrSzI3MnJDWm1rTzRqZFhTZl9NRzZya1hNQUhleTE1MHdQM01fODhVODVJd1ZqVHhHTzhvczluemVzWmdER1hkY3JxcDhEcHNTb2lKU0c2MjZSZVMtZjJzOWRJNlBYNzdhS3N0ZEJJb3JHWllHQUJSdlZuSThBY3RZeXdDZHlTZ19iNWc1U0wyQ1dfajVldWk0V0hkbm1uT3lBcFRJNmxGc3d6TEFOTGt4Q3JDOUVNTGFLb1p6RGtGNDdneG9nLWhmMzFkb1BOT3F2TE9PbTQwT3lGWW5IcjMzUmRyWERJOFJscEc3SFl6ejRUZUFMcUxTemhqVFZvUGVKRUxyNkUwV2ZMSnFvVG1pdGJIbWxIVWc5VUVaSF9WdldLZENrNjVTcVoyclpwVWhNcXl2eUJkZHlZTFVuZlpseXU5VEY0Wm5yc2RtUXBuMnFtcmFpQnBmNnNRZHItS05fSzItQlZyTjNuWG53aTliSkdrS0JJM3AySzYzRXhlb0V1dno1T09JNXlCWjhNWUs2TWVhTFkyVi1EVnQtdGp4SFRuSFZTTnRaUEI3bFoyTmRmc1Z4TVRteExpUVhNUElORTQ977eu77ev\",\"serializedChanges\":\"{\\\"assigned_to\\\":\\\"016ebac82b676210729af47ebe91bf2a\\\",\\\"assignment_group\\\":\\\"3efc509e87dd5910dd76873e8bbb354b\\\"}\",\"chars\":\"c\",\"ignoreRefQual\":false,\"paginationLimit\":25,\"sortBy\":\"name\",\"referenceKey\":null,\"orderByDisplayColumn\":true},\"nowUxInteraction\":\"1er30uhpdjm6-13712\",\"nowUiInteraction\":\"1er30uhpdjm6-25552\",\"cacheable\":false,\"__unstableDisableBatching\":true,\"extensions\":{},\"queryContext\":null}";

wchar_t* sn_headers =
		L"Accept: */*\r\n"
		L"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		L"Accept-Language: en-US,en;q=0.9,en-IE;q=0.8\r\n"
		L"Connection: keep-alive\r\n"
		L"Content-Length: 2266\r\n"
		L"Cookie: %S\r\n"
		// L"Host: digituat.service-now.com\r\n"
		// L"Origin: https://digituat.service-now.com\r\n"
		/* L"Referer: https://digit.service-now.com/now/sow/record/interaction/-1_uid_3\r\n" */
		L"Sec-Fetch-Dest: empty\r\n"
		L"Sec-Fetch-Mode: cors\r\n"
		L"Sec-Fetch-Site: same-origin\r\n"
		L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36 Edg/141.0.0.0\r\n"
		L"content-type: application/json\r\n"
		L"now-ux-interaction: %S \r\n"
		L"sec-ch-ua: \"Microsoft Edge\";v=\"141\", \"Not?A_Brand\";v=\"8\", \"Chromium\";v=\"141\"\r\n"
		L"sec-ch-ua-mobile: ?0\r\n"
		L"sec-ch-ua-platform: \"Windows\"\r\n"
		L"x-request-cancelable: cmgsg9yfv000f2a9541vou7sl-form_section_27d659f353010110b569ddeeff7b12ee_reference_opened_for\r\n"
		L"x-transaction-source: Interface=Web,Interface-Type=Configurable Workspace,Interface-Name=Service Operations Workspace,Interface-SysID=aa881cad73c4301045216238edf6a716\r\n"
	  L"x-usertoken: %S";

wchar_t* raw =
L"Host: digituat.service-now.com\r\n"
L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:143.0) Gecko/20100101 Firefox/143.0\r\n"
L"Accept: */*\r\n"
L"Accept-Language: en-US,en;q=0.5\r\n"
L"Accept-Encoding: gzip, deflate, br, zstd\r\n"
L"Content-Type: application/json\r\n"
L"Content-Length: 2266\r\n"
L"now-ux-interaction: 1er30uhpdjm6-13712\r\n"
L"x-request-cancelable: cmgsg9yfv000f2a9541vou7sl-form_section_27d659f353010110b569ddeeff7b12ee_reference_opened_for\r\n"
L"x-transaction-source: Interface=Web,Interface-Type=Configurable Workspace,Interface-Name=Service Operations Workspace,Interface-SysID=aa881cad73c4301045216238edf6a716,Interface-URI=/now/sow/record/interaction/-1_uid_44\r\n"
L"x-usertoken: 8b5de2a62ba4ba10729af47ebe91bfc7c841b47bd988ee769e7e17c626a0016da100d1c5\r\n"
L"Sec-Fetch-Dest: empty\r\n"
L"Sec-Fetch-Mode: cors\r\n"
L"Sec-Fetch-Site: same-origin\r\n"
L"Connection: keep-alive\r\n"
L"Cookie: glide_user_route=glide.663f86fe884862f3a4671e8cd50bd134; glide_sso_id=ea698ec21b748c10e53dc9506e4bcb8e; BIGipServerpool_digituat=9c7f0f1612bfab18f52b5351949b02c0; JSESSIONID=A579BCA35773E3D90D36E30D5BDE7ECF; glide_node_id_for_js=6020ee0e7cc8e143e7414a5588ada3a1f62e8d5af2c2f34687b86b5e67b0c1e0; glide_language=en; glide_user_activity=U0N2M18xOlZDTVljNnY5aUtpa2xEUnZHVGpreDFQYjl2ZFAyUU1kazAyKzFHQlNkSUk9OmwxWmdUdFJ0WTA0TUI0TVdsSnFXRTQ3akQwTnlZVjY5Mk42WFBib1BwakE9; __CJ_g_startTime=%221760560436722%22; glide_session_store=4B5DE2A62BA4BA10729AF47EBE91BFC7";
#endif // SERVICE_NOW_C
