/******************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/****************************************************************************
 *
 *   Copyright (C) 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#include <tinyara/arch.h>
#include <arch/irq.h>
#include <arch/xtensa/core.h>
#include <arch/chip/core-isa.h>
#include <arch/xtensa/xtensa_corebits.h>
#include <arch/xtensa/xtensa_coproc.h>

#include "xtensa.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_initial_state
 *
 * Description:
 *   A new thread is being started and a new TCB has been created. This
 *   function is called to initialize the processor specific portions of the
 *   new TCB.
 *
 *   This function must setup the intial architecture registers and/or stack
 *   so that execution will begin at tcb->start on the next context switch.
 *
 ****************************************************************************/

void up_initial_state(struct tcb_s *tcb)
{
	struct xcptcontext *xcp = &tcb->xcp;

	/* Initialize the initial exception register context structure */

	memset(xcp, 0, sizeof(struct xcptcontext));

	/* Set initial values of registers */

	xcp->regs[REG_PC] = (uint32_t)tcb->start;	/* Task entrypoint                */
	xcp->regs[REG_A0] = 0;		/* To terminate GDB backtrace     */
	xcp->regs[REG_A1] = (uint32_t)tcb->adj_stack_ptr;	/* Physical top of stack frame    */

	/* Set initial PS to int level 0, EXCM disabled ('rfe' will enable), user
	 * mode.
	 */

#ifdef __XTENSA_CALL0_ABI__
	xcp->regs[REG_PS] = PS_UM | PS_EXCM;

#else
	/* For windowed ABI set WOE and CALLINC (pretend task was 'call4'd). */

	xcp->regs[REG_PS] = PS_UM | PS_EXCM | PS_WOE | PS_CALLINC(1);
#endif

#if XCHAL_CP_NUM > 0
	/* Set up the co-processors that will be enabled initially when the thread
	 * starts (see xtensa_coproc.h).  If the lazy co-processor state restore
	 * logic is selected, that would be the empty set.
	 */

#ifdef CONFIG_XTENSA_CP_LAZY
	xcp->cpstate.cpenable = 0;	/* No co-processors are enabled */
#else
	xcp->cpstate.cpenable = (CONFIG_XTENSA_CP_INITSET & XTENSA_CP_ALLSET);
#endif
	xcp->cpstate.cpstored = 0;	/* No co-processors haved state saved */
#endif
}
