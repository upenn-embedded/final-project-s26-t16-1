[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/-Acvnhrq)
# Final Project

**Team Number:** T16

**Team Name:** Sewing Machine

| Team Member Name | Email Address             |
|------------------|---------------------------|
| Chris Chen       | chrisc05@seas.upenn.edu   |
| Ivy Jiang        | ivyjiang@seas.upenn.edu   |
| Evelyn Li        | eli22@seas.upenn.edu      |
| Daniel Lin       | dl1n@seas.upenn.edu       |

**GitHub Repository URL:** https://github.com/upenn-embedded/final-project-s26-t16-new

**GitHub Pages Website URL:** [for final submission]*

## Final Project Proposal

### 1. Abstract

### 2. Motivation

### 3. System Block Diagram

### 4. Design Sketches

### 5. Software Requirements Specification (SRS)

**5.1 Definitions, Abbreviations**

Here, you will define any special terms, acronyms, or abbreviations you plan to use for hardware

**5.2 Functionality**

| ID     | Description                                                                                                                                                                                                              |
| ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| SRS-01 | The IMU 3-axis acceleration will be measured with 16-bit depth every 100 milliseconds +/-10 milliseconds                                                                                                                 |
| SRS-02 | The distance sensor shall operate and report values at least every .5 seconds.                                                                                                                                           |
| SRS-03 | Upon non-nominal distance detected (i.e., the trap mechanism has changed at least 10 cm from the nominal range), the system shall be able to detect the change and alert the user in a timely manner (within 5 seconds). |
| SRS-04 | Upon a request from the user, the system shall get an image from the internal camera and upload the image to the user system within 10s.                                                                                 |

### 6. Hardware Requirements Specification (HRS)

**6.1 Definitions, Abbreviations**

Here, you will define any special terms, acronyms, or abbreviations you plan to use for hardware

**6.2 Functionality**

| ID     | Description                                                                                                                        |
| ------ | ---------------------------------------------------------------------------------------------------------------------------------- |
| HRS-01 | A distance sensor shall be used for obstacle detection. The sensor shall detect obstacles at a maximum distance of at least 10 cm. |
| HRS-02 | A noisemaker shall be inside the trap with a strength of at least 55 dB.                                                           |
| HRS-03 | An electronic motor shall be used to reset the trap remotely and have a torque of 40 Nm in order to reset the trap mechanism.      |
| HRS-04 | A camera sensor shall be used to capture images of the trap interior. The resolution shall be at least 480p.                       |

### 7. Bill of Materials (BOM)


### 8. Final Demo Goals


### 9. Sprint Planning

| Milestone  | Functionality Achieved | Distribution of Work |
| ---------- | ---------------------- | -------------------- |
| Sprint #1  |                        |                      |
| Sprint #2  |                        |                      |
| MVP Demo   |                        |                      |
| Final Demo |                        |                      |

**This is the end of the Project Proposal section. The remaining sections will be filled out based on the milestone schedule.**

## Sprint Review #1

### Last week's progress
This past week, we finalized BOMed (and order), worked on an intial physical CAD (and sent out to print) and wrote an inital version of the embedded code for the foot pedal system.

In terms of the BOM, the biggest change we made compared to the inital write up was we switchd the force sensor to a different model with a larger surface area for ease of integration and use. We also realized the power adapter we were planning on using does not source enough current to account for spikes in the motor so we made changes to this. 

![alt text](bom_wk1.png)

In terms code for the foot pedal, we have written and tested the functionality for measuring and reading the force from the sensor. This is the core of the control system. 

![alt text](code-wk1.png)

For the physical CAD, we started from example CAD models on the internet and made the necessary adjustments to incorporate our components and detach the foot pedal from the rest of the machine to make our design more convenient and portable. Some of our components that we sent for printing are shone below.

![alt text](cad_photo_wk1.png)

Finally, we met as a team and aligned on direction and plans/goals for next week. 

### Current state of project
Currently, the project is off to a good start. We have a very basic preliminary MVP of our "foot-pedal" system (both circuit and code) which is a core part of the project that will ultimately allow the user to control the sewing machine. We also have a first version for the gears and enclosure of the sewing machine. 

Right now, the biggest bottleneck is just waiting for components to come in. That is, we have ordered electronic parts and are awaiting those to ship. We have also sent out 3D print jobs to RPL and are waiting for the first print to come back so we can iterate. The reason we want to the physical enclosure to come right now 1. printing takes a long time and 2. we want to make sure the elecontric components fit within the mechanical CAD and the gear works as expected. 

In the meantime we are able to progress with components that are in Detkin. Some of these are parts we will use in our final version (feather, LCD screen, Hall sensor etc.) and some are alternate versions we are just using for test (force sensor, small motor)

### Next week's plan
Estimated time: 2 hours 
Assigned: Daniel, Chris 

Finalize foot-pedal subsystem - stress test code, integrate feather for wifi communication, integrate LCD screen displaying status. Being done means have a working system that consistently measures accurate pressue, is able to send it wirelessly and display it visually. Provided 3D print finishes in time, we also want to have this integrated. This group will also be responsible for following up with the 3D print and ensuring all components are printed/accounted for.

Estimated time: 2 hours
Assigned: Evelyn, Ivy

Begin the code and electronics to measure the amount of yarn left/used. This will mean building and testing the hall sensor electroncs and writing the code to support this. We also want to integrate feather for wifi communication on the ATMega connected to this system. Being done means having a hall sensor that can accurate measure rotations and communicte that via wifi. 

*Note: not too much work assigned for this sprint due to midterms and team plans to meet for long work session the Sunday immediately after Sprint 2 is due. 

## Sprint Review #2

### Last week's progress

### Current state of project

### Next week's plan

## MVP Demo

## Final Report

Don't forget to make the GitHub pages public website!
If you’ve never made a GitHub pages website before, you can follow this webpage (though, substitute your final project repository for the GitHub username one in the quickstart guide):  [https://docs.github.com/en/pages/quickstart](https://docs.github.com/en/pages/quickstart)

### 1. Video

### 2. Images

### 3. Results

#### 3.1 Software Requirements Specification (SRS) Results

| ID     | Description                                                                                               | Validation Outcome                                                                          |
| ------ | --------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| SRS-01 | The IMU 3-axis acceleration will be measured with 16-bit depth every 100 milliseconds +/-10 milliseconds. | Confirmed, logged output from the MCU is saved to "validation" folder in GitHub repository. |

#### 3.2 Hardware Requirements Specification (HRS) Results

| ID     | Description                                                                                                                        | Validation Outcome                                                                                                      |
| ------ | ---------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| HRS-01 | A distance sensor shall be used for obstacle detection. The sensor shall detect obstacles at a maximum distance of at least 10 cm. | Confirmed, sensed obstacles up to 15cm. Video in "validation" folder, shows tape measure and logged output to terminal. |
|        |                                                                                                                                    |                                                                                                                         |

### 4. Conclusion


## References

