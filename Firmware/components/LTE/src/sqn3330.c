// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifdef CONFIG_NETWORK_PPP
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "sqn3330.h"

/**
 * @brief Macro defined for error checking
 *
 */
static const char *DCE_TAG = "sqn3330";
#define DCE_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                \
    {                                                                                 \
        if (!(a))                                                                     \
        {                                                                             \
            ESP_LOGE(DCE_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                            \
        }                                                                             \
    } while (0)

/**
 * @brief SQN3330 Modem
 *
 */
typedef struct {
    void *priv_resource; /*!< Private resource */
    modem_dce_t parent;  /*!< DCE parent class */
} sqn3330_modem_dce_t;

/**
 * @brief Handle response from AT+CSQ
 */
static esp_err_t sqn3330_handle_csq(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    sqn3330_modem_dce_t *sqn3330_dce = __containerof(dce, sqn3330_modem_dce_t, parent);
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CSQ", strlen("+CSQ"))) {
        /* store value of rssi and ber */
        uint32_t **csq = sqn3330_dce->priv_resource;
        /* +CSQ: <rssi>,<ber> */
        sscanf(line, "%*s%d,%d", csq[0], csq[1]);
        
        /* mapping values */
        if (*csq[0] == 0)
            *csq[0] = 113;
        else if (*csq[0] == 1)
            *csq[0] = 111;
        else if ((*csq[0] > 1) && (*csq[0] < 31))
            *csq[0] = ((53 - 109) / (30 - 2)*(*csq[0] - 2) + 109);
        else if (*csq[0] == 31)
            *csq[0] = 51;
        else if (*csq[0] == 99)
            *csq[0] = 1000;
        err = ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from +++
 */
static esp_err_t sqn3330_handle_exit_data_mode(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_NO_CARRIER)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        err = ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from ATD*99#
 */
static esp_err_t sqn3330_handle_atd_ppp(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_CONNECT)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

/**
 * @brief Handle network registration response
 */
static esp_err_t sqn3330_handle_cereg(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = ESP_OK;
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = (dce->state == MODEM_STATE_PROCESSING)?esp_modem_process_command_done(dce, MODEM_STATE_FAIL):ESP_OK;
    } else if (strstr(line, "+CEREG: 0")) {
        ESP_LOGI(DCE_TAG, "MT Not Registered, Not Searching");
        dce->link = MT_NOT_REGISTERED;
        err = ESP_OK;
    } else if (strstr(line, "+CEREG: 2")) {
        ESP_LOGI(DCE_TAG, "MT Not Registered, Searching");
        dce->link = MT_SEARCHING;
        err = ESP_OK;
    } else if (strstr(line, "+CEREG: 4")) {
        ESP_LOGI(DCE_TAG, "MT Unknown");
        dce->link = MT_UNKNOWN;
        err = ESP_OK;
    } else if (strstr(line, "+CEREG: 5")) {
        ESP_LOGI(DCE_TAG, "MT Registered, Roaming");
        dce->link = MT_ROAMING;
        err = (dce->state == MODEM_STATE_PROCESSING)?esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS):ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from AT^RESET
 */
static esp_err_t sqn3330_handle_reset(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = ESP_OK;
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (strstr(line, "+SHUTDOWN")) {
        err = ESP_OK;
    } else if (strstr(line, "+SYSSTART")) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    }
    return err;
}

/**
 * @brief Handle response from AT+CFUN
 */
static esp_err_t sqn3330_handle_func(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

/**
 * @brief Handle response from AT+CGMM
 */
static esp_err_t sqn3330_handle_cgmm(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->name, MODEM_MAX_NAME_LENGTH, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->name, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+CGSN
 */
static esp_err_t sqn3330_handle_cgsn(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->imei, MODEM_IMEI_LENGTH + 1, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->imei, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+CIMI
 */
static esp_err_t sqn3330_handle_cimi(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->imsi, MODEM_IMSI_LENGTH + 1, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->imsi, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+COPS?
 */
static esp_err_t sqn3330_handle_cops(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+COPS", strlen("+COPS"))) {
        /* there might be some random spaces in operator's name, we can not use sscanf to parse the result */
        /* strtok will break the string, we need to create a copy */
        size_t len = strlen(line);
        char *line_copy = malloc(len + 1);
        strcpy(line_copy, line);
        /* +COPS: <mode>[, <format>[, <oper>]] */
        char *str_ptr = NULL;
        char *p[3];
        uint8_t i = 0;
        /* strtok will broke string by replacing delimiter with '\0' */
        p[i] = strtok_r(line_copy, ",", &str_ptr);
        while (p[i]) {
            p[++i] = strtok_r(NULL, ",", &str_ptr);
        }
        if (i >= 3) {
            int len = snprintf(dce->oper, MODEM_MAX_OPERATOR_LENGTH, "%s", p[2]);
            if (len > 2) {
                /* Strip "\r\n" */
                strip_cr_lf_tail(dce->oper, len);
                err = ESP_OK;
            }
        }
        free(line_copy);
    }
    return err;
}

/**
 * @brief Get signal quality
 *
 * @param dce Modem DCE object
 * @param rssi received signal strength indication
 * @param ber bit error ratio
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_get_signal_quality(modem_dce_t *dce, uint32_t *rssi, uint32_t *ber)
{
    ESP_LOGI(DCE_TAG, "Signal quality [AT+CSQ]");
    modem_dte_t *dte = dce->dte;
    sqn3330_modem_dce_t *sqn3330_dce = __containerof(dce, sqn3330_modem_dce_t, parent);
    uint32_t *resource[2] = {rssi, ber};
    sqn3330_dce->priv_resource = resource;
    dce->handle_line = sqn3330_handle_csq;
    DCE_CHECK(dte->send_cmd(dte, "AT+CSQ\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "inquire signal quality failed", err);
    ESP_LOGD(DCE_TAG, "inquire signal quality ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Set Working Mode
 *
 * @param dce Modem DCE object
 * @param mode woking mode
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_set_working_mode(modem_dce_t *dce, modem_mode_t mode)
{
    modem_dte_t *dte = dce->dte;
    switch (mode) {
    case MODEM_COMMAND_MODE:
        dce->handle_line = sqn3330_handle_exit_data_mode;
        DCE_CHECK(dte->send_cmd(dte, "+++", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
        DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "enter command mode failed", err);
        ESP_LOGD(DCE_TAG, "enter command mode ok");
        dce->mode = MODEM_COMMAND_MODE;
        break;
    case MODEM_PPP_MODE:
        dce->handle_line = sqn3330_handle_atd_ppp;
        DCE_CHECK(dte->send_cmd(dte, "ATD*99***1#\r", MODEM_COMMAND_TIMEOUT_MODE_CHANGE) == ESP_OK, "send command failed", err);
        DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "enter ppp mode failed", err);
        ESP_LOGD(DCE_TAG, "enter ppp mode ok");
        dce->mode = MODEM_PPP_MODE;
        break;
    default:
        ESP_LOGW(DCE_TAG, "unsupported working mode: %d", mode);
        goto err;
        break;
    }
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Reset DCE module and disable RF circuitry
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_reset_module(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "Reset [AT^RESET]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_reset;
    DCE_CHECK(dte->send_cmd(dte, "AT+CFUN=4,1\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "reset module failed", err);
    ESP_LOGD(DCE_TAG, "reset module ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Enable DCE module RF circuitry
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_set_functionality(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "Functionality [AT+CFUN]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_func;
    DCE_CHECK(dte->send_cmd(dte, "AT+CFUN=1\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "enable RF failed", err);
    ESP_LOGD(DCE_TAG, "enable RF ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Select ISP Operator
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_set_operator_name(sqn3330_modem_dce_t *sqn3330_dce)
{
    char pdp[128];
    ESP_LOGI(DCE_TAG, "Automatic Operator Selection [AT+COPS]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_cereg;
    DCE_CHECK(dte->send_cmd(dte, "", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err_init);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "operator not found", err_init);
    ESP_LOGD(DCE_TAG, "operator found");
    return ESP_OK;
err_init:
    ESP_LOGI(DCE_TAG, "Manual Operator Selection [AT+COPS]");
    sqn3330_dce->parent.handle_line = sqn3330_handle_cereg;
    snprintf(pdp, sizeof(pdp), "AT+COPS=1,2,\"%s\"\r", CONFIG_ISP_NOC);
    DCE_CHECK(dte->send_cmd(dte, pdp, MODEM_COMMAND_TIMEOUT_OPERATOR) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "operator not found", err);
    ESP_LOGD(DCE_TAG, "operator found");
    return ESP_OK;
err:
    sqn3330_dce->parent.handle_line = sqn3330_handle_cereg;
    return ESP_FAIL;
}

/**
 * @brief Get DCE module name
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_get_module_name(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "Name [AT+CGMM]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_cgmm;
    DCE_CHECK(dte->send_cmd(dte, "AT+CGMM\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "get module name failed", err);
    ESP_LOGD(DCE_TAG, "get module name ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get DCE module IMEI number
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_get_imei_number(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "IMEI [AT+CGSN]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_cgsn;
    DCE_CHECK(dte->send_cmd(dte, "AT+CGSN\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "get imei number failed", err);
    ESP_LOGD(DCE_TAG, "get imei number ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get DCE module IMSI number
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_get_imsi_number(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "IMSI [AT+CIMI]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_cimi;
    DCE_CHECK(dte->send_cmd(dte, "AT+CIMI\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "get imsi number failed", err);
    ESP_LOGD(DCE_TAG, "get imsi number ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get Operator's name
 *
 * @param sqn3330_dce sqn3330 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sqn3330_get_operator_name(sqn3330_modem_dce_t *sqn3330_dce)
{
    ESP_LOGI(DCE_TAG, "Get operator [AT+COPS?]");
    modem_dte_t *dte = sqn3330_dce->parent.dte;
    sqn3330_dce->parent.handle_line = sqn3330_handle_cops;
    DCE_CHECK(dte->send_cmd(dte, "AT+COPS?\r", MODEM_COMMAND_TIMEOUT_OPERATOR) == ESP_OK, "send command failed", err);
    DCE_CHECK(sqn3330_dce->parent.state == MODEM_STATE_SUCCESS, "get network operator failed", err);
    ESP_LOGD(DCE_TAG, "get network operator ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Deinitialize SQN3330 object
 *
 * @param dce Modem DCE object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on fail
 */
static esp_err_t sqn3330_deinit(modem_dce_t *dce)
{
    sqn3330_modem_dce_t *sqn3330_dce = __containerof(dce, sqn3330_modem_dce_t, parent);
    if (dce->dte) {
        dce->dte->dce = NULL;
    }
    free(sqn3330_dce);
    return ESP_OK;
}

modem_dce_t *sqn3330_init(modem_dte_t *dte)
{
    DCE_CHECK(dte, "DCE should bind with a DTE", err);
    /* malloc memory for sqn3330_dce object */
    sqn3330_modem_dce_t *sqn3330_dce = calloc(1, sizeof(sqn3330_modem_dce_t));
    DCE_CHECK(sqn3330_dce, "calloc sqn3330_dce failed", err);
    /* Bind DTE with DCE */
    sqn3330_dce->parent.dte = dte;
    dte->dce = &(sqn3330_dce->parent);
    /* Bind methods */
    sqn3330_dce->parent.handle_line = NULL;
    sqn3330_dce->parent.sync = esp_modem_dce_sync;
    sqn3330_dce->parent.echo_mode = esp_modem_dce_echo;
    sqn3330_dce->parent.store_profile = esp_modem_dce_store_profile;
    sqn3330_dce->parent.set_flow_ctrl = esp_modem_dce_set_flow_ctrl;
    sqn3330_dce->parent.define_pdp_context = esp_modem_dce_define_pdp_context;
    sqn3330_dce->parent.hang_up = esp_modem_dce_hang_up;
    sqn3330_dce->parent.get_signal_quality = sqn3330_get_signal_quality;
    sqn3330_dce->parent.get_battery_status = NULL;
    sqn3330_dce->parent.set_working_mode = sqn3330_set_working_mode;
    sqn3330_dce->parent.power_down = NULL;
    sqn3330_dce->parent.deinit = sqn3330_deinit;

    /* Sync between DTE and DCE */
    DCE_CHECK(esp_modem_dce_sync(&(sqn3330_dce->parent)) == ESP_OK, "sync failed", err_io);
    
    /* Close echo */
    DCE_CHECK(esp_modem_dce_echo(&(sqn3330_dce->parent), false) == ESP_OK, "close echo mode failed", err_io);
    
    /* Reset Module and disable RF*/
    DCE_CHECK(sqn3330_reset_module(sqn3330_dce) == ESP_OK, "reset module failed", err_io);
    
    /* Get Module name */
    DCE_CHECK(sqn3330_get_module_name(sqn3330_dce) == ESP_OK, "get module name failed", err_io);
    
    /* Get IMEI number */
    DCE_CHECK(sqn3330_get_imei_number(sqn3330_dce) == ESP_OK, "get imei failed", err_io);
    
    /* Get IMSI number */
    DCE_CHECK(sqn3330_get_imsi_number(sqn3330_dce) == ESP_OK, "get imsi failed", err_io);

    /* Set PDP Context */
    DCE_CHECK(esp_modem_dce_define_pdp_context(&(sqn3330_dce->parent), 1, "IP", CONFIG_ISP_APN) == ESP_OK, "set modem APN failed", err_io);

    /* Set full functionality */
    DCE_CHECK(sqn3330_set_functionality(sqn3330_dce) == ESP_OK, "enable RF failed", err_io);

    /* Set operator name */
    DCE_CHECK(sqn3330_set_operator_name(sqn3330_dce) == ESP_OK, "set operator name failed", err_io);

    /* Get operator name */
    DCE_CHECK(sqn3330_get_operator_name(sqn3330_dce) == ESP_OK, "get operator name failed", err_io);

    return &(sqn3330_dce->parent);
err_io:
    free(sqn3330_dce);
err:
    return NULL;
}
#endif
