response_t power_response()
{
    return common_response();
}

void init_firehose_power(char *act, firehose_power_t *power)
{
    memcpy(power->value, act, sizeof(power->value));
    char *format = XML_HEADER
                   "<data><power value=\"%s\" "
                   "delayinseconds=\"2\" /></data>";
    sprintf(power->xml, format, act);
    send_command(power->xml, strlen(power->xml));
}

int send_firehose_power(firehose_power_t power)
{
    return send_command(power.xml, strlen(power.xml));
}

response_t process_power_action(char *act)
{
    firehose_power_t power;
    memset(&power, 0, sizeof(firehose_power_t));
    init_firehose_power(act, &power);
    send_firehose_power(&power);  
    return  power_response();
}
