/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 */
package org.contikios.cooja.contikimote.interfaces;


import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;


import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Observable;
import java.util.Observer;
import java.lang.*;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.contikios.cooja.COOJARadioPacket;
import org.contikios.cooja.Mote;
import org.contikios.cooja.RadioPacket;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.contikimote.ContikiMote;
import org.contikios.cooja.interfaces.PolledAfterActiveTicks;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.mote.memory.VarMemory;
import org.contikios.cooja.radiomediums.UDGM;
import org.contikios.cooja.util.CCITT_CRC;




import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.PolledAfterAllTicks;
import org.contikios.cooja.interfaces.PolledBeforeAllTicks;
import org.contikios.cooja.mote.memory.SectionMoteMemory;
/**
 * An example of how to implement new mote interfaces.
 *
 * Contiki variables:
 * <ul>
 * <li>char simDummyVar
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>dummy_interface
 * </ul>
 * <p>
 *
 * This observable never changes.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Dummy Interface")
public class DummyInterface extends Radio implements ContikiMoteInterface, PolledBeforeAllTicks, PolledAfterAllTicks {
  private static Logger logger = Logger.getLogger(DummyInterface.class);

  private ContikiMote mote;
  private VarMemory myMoteMemory;


/**
   * Transmission bitrate (kbps).
   */
  private double RADIO_TRANSMISSION_RATE_kbps;

  private RadioPacket packetToMote = null;

  private RadioPacket packetFromMote = null;

  private boolean radioOn = true;

  private boolean isTransmitting = false;

  private boolean isInterfered = false;

  private long transmissionEndTime = -1;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private long lastEventTime = 0;

  private int oldOutputPowerIndicator = -1;

  private int oldRadioChannel = -1;






  public DummyInterface(Mote mote) {
RADIO_TRANSMISSION_RATE_kbps = mote.getType().getConfig().getDoubleValue(
        DummyInterface.class, "RADIO_TRANSMISSION_RATE_kbps");


    this.mote = (ContikiMote) mote;
    this.myMoteMemory = new VarMemory(mote.getMemory());

    radioOn = myMoteMemory.getByteValueOf("simRadioHWOnDummy") == 1;
  }

  public static String[] getCoreInterfaceDependencies() {
    // I need the corresponding C dummy interface (in dummy_intf.c)
    return new String[] { "dummy_interface" };
  }

  public void doActionsBeforeTick() {
    logger.debug("Java-part of dummy interface acts BEFORE mote tick:");
  }

  public void doActionsAfterTick() {
   
    long now = mote.getSimulation().getSimulationTime();

    /* Check if radio hardware status changed */
    if (radioOn != (myMoteMemory.getByteValueOf("simRadioHWOnDummy") == 1)) {
      radioOn = !radioOn;

      if (!radioOn) {
        myMoteMemory.setByteValueOf("simReceivingDummy", (byte) 0);
        myMoteMemory.setIntValueOf("simInSizeDummy", 0);
        myMoteMemory.setIntValueOf("simOutSizeDummy", 0);
        isTransmitting = false;
        lastEvent = RadioEvent.HW_OFF;
      } else {
        lastEvent = RadioEvent.HW_ON;
      }

      lastEventTime = now;
      this.setChanged();
      this.notifyObservers();
    }
    if (!radioOn) {
      return;
    }

    /* Check if radio output power changed */
    if (myMoteMemory.getByteValueOf("simPowerDummy") != oldOutputPowerIndicator) {
      oldOutputPowerIndicator = myMoteMemory.getByteValueOf("simPowerDummy");
      lastEvent = RadioEvent.UNKNOWN;
      this.setChanged();
      this.notifyObservers();
    }

    /* Check if radio channel changed */
    if (getChannel() != oldRadioChannel) {
      oldRadioChannel = getChannel();
      lastEvent = RadioEvent.UNKNOWN;
      this.setChanged();
      this.notifyObservers();
    }

    /* Ongoing transmission */
    if (isTransmitting && now >= transmissionEndTime) {
      myMoteMemory.setIntValueOf("simOutSizeDummy", 0);
      isTransmitting = false;
      mote.requestImmediateWakeup();

      lastEventTime = now;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      this.setChanged();
      this.notifyObservers();
      /*logger.debug("----- CONTIKI TRANSMISSION ENDED -----");*/
    }

    /* New transmission */
    int size = myMoteMemory.getIntValueOf("simOutSizeDummy");
    if (!isTransmitting && size > 0) {
      packetFromMote = new COOJARadioPacket(myMoteMemory.getByteArray("simOutDataBufferDummy", size + 2));

      if (packetFromMote.getPacketData() == null || packetFromMote.getPacketData().length == 0) {
        logger.warn("Skipping zero sized Contiki packet (no buffer)");
        myMoteMemory.setIntValueOf("simOutSizeDummy", 0);
        mote.requestImmediateWakeup();
        return;
      }

      byte[] data = packetFromMote.getPacketData();
      CCITT_CRC txCrc = new CCITT_CRC();
      txCrc.setCRC(0);
      for (int i = 0; i < size; i++) {
        txCrc.addBitrev(data[i]);
      }
      data[size] = (byte)txCrc.getCRCHi();
      data[size + 1] = (byte)txCrc.getCRCLow();

      isTransmitting = true;

      /* Calculate transmission duration (us) */
      /* XXX Currently floored due to millisecond scheduling! */
      long duration = (int) (Simulation.MILLISECOND*((8 * size /*bits*/) / RADIO_TRANSMISSION_RATE_kbps));
      transmissionEndTime = now + Math.max(1, duration);

      lastEventTime = now;
      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      this.setChanged();
      this.notifyObservers();
      //logger.debug("----- NEW CONTIKI TRANSMISSION DETECTED -----");

      // Deliver packet right away
      lastEvent = RadioEvent.PACKET_TRANSMITTED;
      this.setChanged();
      this.notifyObservers();
      //logger.debug("----- CONTIKI PACKET DELIVERED -----");
    }

    if (isTransmitting && transmissionEndTime > now) {
      mote.scheduleNextWakeup(transmissionEndTime);
    }
  }

 /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return packetFromMote;
  }

  public RadioPacket getLastPacketReceived() {
    return packetToMote;
  }

  public void setReceivedPacket(RadioPacket packet) {
    packetToMote = packet;
  }

  /* General radio support */
  public boolean isRadioOn() {
    return radioOn;
  }

  public boolean isTransmitting() {
    return isTransmitting;
  }

  public boolean isReceiving() {
    return myMoteMemory.getByteValueOf("simReceivingDummy") == 1;
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public int getChannel() {
    return myMoteMemory.getIntValueOf("simRadioChannelDummy");
  }
public void signalReceptionStart() {
    packetToMote = null;
    if (isInterfered() || isReceiving() || isTransmitting()) {
      interfereAnyReception();
      return;
    }

    myMoteMemory.setByteValueOf("simReceivingDummy", (byte) 1);
    mote.requestImmediateWakeup();

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;

    myMoteMemory.setInt64ValueOf("simLastPacketTimestampDummy", lastEventTime);

    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    if (isInterfered || packetToMote == null) {
      isInterfered = false;
      packetToMote = null;
      myMoteMemory.setIntValueOf("simInSizeDummy", 0);
    } else {
      myMoteMemory.setIntValueOf("simInSizeDummy", packetToMote.getPacketData().length - 2);
      myMoteMemory.setByteArray("simInDataBufferDummy", packetToMote.getPacketData());
    }

    myMoteMemory.setByteValueOf("simReceivingDummy", (byte) 0);
    mote.requestImmediateWakeup();
    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    this.setChanged();
    this.notifyObservers();
  }
public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    if (isInterfered()) {
      return;
    }
 
    isInterfered = true;

    lastEvent = RadioEvent.RECEPTION_INTERFERED;
    lastEventTime = mote.getSimulation().getSimulationTime();
    this.setChanged();
    this.notifyObservers();
  }

  public double getCurrentOutputPower() {
    /* TODO Implement method */
    
    return 0;
  }

  public int getOutputPowerIndicatorMax() {
    return 100;
  }

  public int getCurrentOutputPowerIndicator() {
    return myMoteMemory.getByteValueOf("simPowerDummy");
  }

  public double getCurrentSignalStrength() {
    return myMoteMemory.getIntValueOf("simSignalStrengthDummy");
    
     
  }

  public void setCurrentSignalStrength(double signalStrength) {
    myMoteMemory.setIntValueOf("simSignalStrengthDummy", (int) signalStrength);
  }
public void setLQI(int lqi){
    if(lqi<0) {
      lqi=0;
    }
    else if(lqi>0xff) {
      lqi=0xff;
    }
    myMoteMemory.setIntValueOf("simLQIDummy", lqi);
  }

  public int getLQI(){
    return myMoteMemory.getIntValueOf("simLQIDummy");
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }




  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel(new BorderLayout());
    Box box = Box.createVerticalBox();

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("DummyRadio");

    box.add(statusLabel);
    box.add(lastEventLabel);
    box.add(ssLabel);
    box.add(updateButton);
    box.add(channelLabel);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ssLabel.setText("Signal strength (not auto-updated): "
            + String.format("%1.1f", getCurrentSignalStrength()) + " dBm");
      }
    });

    final Observer observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("Transmitting");
        } else if (isReceiving()) {
          statusLabel.setText("Receiving");
        } else {
          statusLabel.setText("Listening");
        }

        lastEventLabel.setText("Last event: " + getLastEvent());
        ssLabel.setText("Signal strength (not auto-updated): "
            + String.format("%1.1f", getCurrentSignalStrength()) + " dBm");
        if (getChannel() == -1) {
          channelLabel.setText("Current channel: ALL");
        } else {
          channelLabel.setText("Current channel: " + getChannel());
        }
      }
    };
    this.addObserver(observer);

    observer.update(null, null);

    panel.add(BorderLayout.NORTH, box);
    panel.putClientProperty("intf_obs", observer);
    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public Collection<Element> getConfigXML() {
   ArrayList<Element> config = new ArrayList<Element>();

           Element element;

           /* Radio bitrate */
           element = new Element("bitrate");
           element.setText("" + RADIO_TRANSMISSION_RATE_kbps);
           config.add(element);

           return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
 for (Element element : configXML) {
                 if (element.getName().equals("bitrate")) {
                         RADIO_TRANSMISSION_RATE_kbps = Double.parseDouble(element.getText());
                         logger.info("Radio bitrate reconfigured to (kbps): " + RADIO_TRANSMISSION_RATE_kbps);
                 }
         }
  }
public Mote getMote() {
    return mote;
  }

  public String toString() {
    return "DummyInterface";
  }

}
