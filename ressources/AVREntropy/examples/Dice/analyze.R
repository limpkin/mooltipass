# Analyze dice data to establish if it is random
setwd("~/Development/hardware-rng/avr-hardware-random-number-generation/Entropy/examples/Dice")
library(foreign)
library(prettyR)
sink("analysis.prn")
Dice.data <- read.csv("dice.txt")
Dice.data$Sum <- Dice.data$Die1 + Dice.data$Die2
png(filename="cumulative.png", width=800, height=600)
hist(Dice.data$Sum, 
     freq=FALSE, 
     breaks=c(1.5,2.5,3.5,4.5,5.5,6.5,7.5,8.5,9.5,10.5,11.5,12.5),
     main="Distribution of Dice rolls",
     xlab="Combined value",
     ylab="Probability",
     ylim=c(0.00,0.20))
dev.off()
Dice.data.freq <- freq(Dice.data, display.na=FALSE,decr.order=FALSE)
Dice.data.freq <-as.data.frame(Dice.data.freq["Sum"])
print(Dice.data.freq)
events <- sum(Dice.data.freq$Sum)
print(chisq.test(Dice.data.freq$Sum, p=c(0.0278,0.0556,0.0833,0.1111,0.1389,0.1666,0.1389,0.1111,0.0833,0.0556,0.0278)))
Independent.Rolls <- c(Dice.data$Die1,Dice.data$Die2)
png(filename="independent.png",width=800,height=600)
hist(Independent.Rolls, 
     freq=FALSE, 
     breaks=c(0.5,1.5,2.5,3.5,4.5,5.5,6.5),
     main="Distribution of Dice rolls",
     xlab="Independent values",
     ylab="Probability",
     ylim=c(0.00,0.20))
dev.off()
Independent.Rolls.freq <- freq(Independent.Rolls, display.na=FALSE,decr.order=FALSE)
Independent.Rolls.freq <- as.data.frame(Independent.Rolls.freq["Independent.Rolls"])
print(Independent.Rolls.freq)
print(chisq.test(Independent.Rolls.freq))
sink()