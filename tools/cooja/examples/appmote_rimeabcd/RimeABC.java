/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */

package org.contikios.cooja.motes;

import java.awt.Container;

import org.apache.log4j.Logger;

import org.contikios.cooja.AbstractionLevelDescription;
import org.contikios.cooja.COOJARadioPacket;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteTimeEvent;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.RadioPacket;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.MoteType.MoteTypeCreationException;
import org.contikios.cooja.interfaces.ApplicationRadio;
import org.contikios.cooja.interfaces.Radio.RadioEvent;

/**
 * Example application-level mote.
 * Imitates the same behavior as /examples/rime/example-abc.c.
 * 
 * This mote is simulated in COOJA via the Imported App Mote Type.
 * 
 * @author Fredrik Osterlind
 */
public class RimeABC extends AbstractApplicationMote {

  private Simulation simulation = null;
  private ApplicationRadio radio = null;
  static int channel =1;

  private final static byte RIME_CHANNEL1 = (byte) 128;
  private final static byte RIME_CHANNEL2 = (byte) 0;
  private final static RadioPacket RIME_ABC_DATA =
    new COOJARadioPacket(new byte[] {
        RIME_CHANNEL1, RIME_CHANNEL2, 
        'H', 'e', 'l', 'l', 'o', '\0'
    });

  public RimeABC() {
    super();
  }

  public RimeABC(MoteType moteType, Simulation simulation) {
    super(moteType, simulation);
  }

  public void execute(long time) {
    /*System.out.println(this + ": execute(" + time + ")");*/

    if (radio == null) {
      simulation = getSimulation();
      radio = (ApplicationRadio) getInterfaces().getRadio();
      radio.setChannel(18);
    }
 
  

    
    /* Delay 2-4 seconds */
  radio.startTransmittingPacket(
          RIME_ABC_DATA, 10*Simulation.MILLISECOND);
  }

  private MoteTimeEvent transmitEvent = new MoteTimeEvent(this, 0) {
    public void execute(long t) {
      /* Transmit Rime abc radio packet.
       * sentPacket is called when transmission finishes. */
      radio.startTransmittingPacket(
          RIME_ABC_DATA, 10*Simulation.MILLISECOND);

    
    }
  };

  public void receivedPacket(RadioPacket p) {

  }

  public void sentPacket(RadioPacket p) {

    requestImmediateWakeup(); /* Triggers execute method */
  }

  public String toString() {
    return "App Rime ABC " + getID();
  }
}

