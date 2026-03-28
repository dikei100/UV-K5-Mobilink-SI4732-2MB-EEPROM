/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <stddef.h>
#include <string.h>

#include "driver/eeprom.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "settings.h"

bool gEepromWrite = false;

static uint8_t tmpBuffer[256];

void EEPROM_ReadBuffer(uint32_t Address, void *pBuffer, uint16_t Size)
{
	uint8_t IIC_ADD = (uint8_t)(0xA0 | ((Address >> 16) << 1));

	I2C_Start();
	I2C_Write(IIC_ADD);
	I2C_Write((Address >> 8) & 0xFF);
	I2C_Write((Address >> 0) & 0xFF);
	I2C_Start();
	I2C_Write(IIC_ADD + 1);
	I2C_ReadBuffer(pBuffer, Size);
	I2C_Stop();
}

void EEPROM_WriteBuffer(uint32_t Address, const void *pBuffer, uint16_t Size)
{
	if (pBuffer == NULL)
		return;

	const uint16_t PAGE_SIZE = SETTINGS_GetPageSize();
	const uint8_t *pData = (const uint8_t *)pBuffer;

	while (Size) {
		uint16_t i    = Address % PAGE_SIZE;
		uint16_t rest = PAGE_SIZE - i;
		uint16_t n    = Size < rest ? Size : rest;

		EEPROM_ReadBuffer(Address, tmpBuffer, n);
		if (memcmp(pData, tmpBuffer, n) != 0) {
			uint8_t IIC_ADD = (uint8_t)(0xA0 | ((Address >> 16) << 1));

			I2C_Start();
			I2C_Write(IIC_ADD);
			I2C_Write((Address >> 8) & 0xFF);
			I2C_Write((Address >> 0) & 0xFF);
			I2C_WriteBuffer(pData, n);
			I2C_Stop();

			SYSTEM_DelayMs(10);
		}

		pData   += n;
		Address += n;
		Size    -= n;
		gEepromWrite = true;
	}
}
