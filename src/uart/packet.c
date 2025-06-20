#include "uart/packet.h"

/**
 * @brief 向現有 UART 封包中新增資料
 *        Add data to existing UART packet
 *
 * @param self 指向要新增資料的 UART 封包 (input packet)
 * @param vec_u8 要新增的資料向量 (input data vector)
 */
FnState uart_pkt_add_data(UartPacket *self, VecU8 *vec_u8)
{
    FNS_ERROR_CHECK(vec_u8_realign(vec_u8));
    FNS_ERROR_CHECK(vec_u8_push(&self->vec, vec_u8->data, vec_u8->len));
    return FNS_OK;
}

/**
 * @brief 從 UART 封包中取出資料向量 (Extract payload data from UART packet)
 *
 * 從輸入的 UartPacket 取得其內部儲存的資料向量 (vec)，
 * 並回傳該 VecU8 實例。並不包含起始與結束碼 (start/end codes)。
 *
 * @param self  來源 UART 封包指標 (input UART packet pointer)
 * @return     VecU8 由封包提取出的資料向量 (the data vector extracted from the packet)
 */
FnState uart_pkt_get_data(const UartPacket *self, VecU8 *vec_u8)
{
    FNS_ERROR_CHECK(vec_u8_rm_all(vec_u8));
    FNS_ERROR_CHECK(vec_u8_push(vec_u8, self->vec.data, self->vec.len));
    return FNS_OK;
}

/**
 * @brief 根據原始資料向量打包成 UART 封包，並移除起始與結束碼後重新封裝
 *        Pack raw data vector into UART packet, stripping start and end codes before repacking
 *
 * @param self 輸出參數，接收封裝後的 UART 封包 (output packed UART packet)
 * @param vec_u8 包含封包起始碼與結束碼的資料向量 (input byte vector with start/end codes)
 * @return bool 是否封包成功 (true if pack successful, false otherwise)
 */
FnState uart_pkt_pack(UartPacket *self, VecU8 *vec_u8)
{
    uint8_t byte;
    FNS_ERROR_CHECK(vec_u8_get_byte(vec_u8, &byte, 0));
    if (byte != PACKET_START_CODE) return FNS_NO_MATCH;
    FNS_ERROR_CHECK(vec_u8_get_byte(vec_u8, &byte, vec_u8->len - 1));
    if (byte != PACKET_END_CODE) return FNS_NO_MATCH;
    FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, 1));
    FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, vec_u8->len-1, 1));
    FNS_ERROR_CHECK(vec_u8_realign(vec_u8));
    self->vec = *vec_u8;
    return FNS_OK;
}

/**
 * @brief 解包 UART 封包，將起始碼、資料與結束碼合併為一個資料向量
 *        Unpack UART packet into a byte vector including start, data, and end codes
 *
 * @param self 指向要解包的 UART 封包 (input packet)
 * @return VecU8 包含完整封包的資料向量 (vector containing full packet bytes)
 */
FnState uart_pkt_unpack(const UartPacket *self, VecU8 *vec_u8)
{
    FNS_ERROR_CHECK(vec_u8_rm_all(vec_u8));
    FNS_ERROR_CHECK(vec_u8_push_byte(vec_u8, self->start));
    FNS_ERROR_CHECK(vec_u8_push(vec_u8, self->vec.data, self->vec.len));
    FNS_ERROR_CHECK(vec_u8_push_byte(vec_u8, self->end));
    return FNS_OK;
}
