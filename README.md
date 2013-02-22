YodleJugglefest
===============

Solution to [Yodle Jugglesfest] (http://www.yodlecareers.com/puzzles/jugglefest.html) challenge.

 Problem

 Each team will attempt to complete a juggling circuit consisting of several tricks. 
 Each circuit emphasizes different aspects of juggling, 
 requiring hand to eye coordination (H), endurance (E) and pizzazz (P) in various amounts for successful completion.
 Each juggler has these abilities in various amounts as well. 
 How good a match they are for a circuit is determined by 
 the dot product of the juggler's and the circuit's H, E, and P values. 
 The higher the result, the better the match.

 Each participant will be on exactly one team and there will be a distinct circuit for each team to attempt.
 Each participant will rank in order of preference their top X circuits. Since we would like the audiences to 
 enjoy the performances as much as possible, when assigning jugglers to circuits we also want to consider 
 how well their skills match up to the circuit. In fact we want to match jugglers to circuits such that 
 no juggler could switch to a circuit that they prefer more than the one they are assigned to and 
 be a better fit for that circuit than one of the other jugglers assigned to it.

 To help us create the juggler/circuit assignments write a program in a language of your choice that takes as
 input a file of circuits and jugglers and outputs a file of circuits and juggler assignments. 
 The number of jugglers assigned to a circuit should be the number of jugglers divided by the number of circuits. 
 Assume that the number of circuits and jugglers will be such that each circuit will have the same number of jugglers
 with no remainder.

 Solution

 >> There will be thousands of participants split into teams.
 
 Thousands of participants, each one has a preferences list which might cover thousands of teams.
 So size of the problem is big enough to stay away from quadratic space complexity, but we have some room for 
 constant term in linear complexity. We'll keep that in mind if it comes to trade offs between 
 space and run time efficiency, also we can trade some efficiency for code simplicity. 

 >> we want to match jugglers to circuits such that 
 >> no juggler could switch to a circuit that they prefer more than the one they are assigned to
 >> and be a better fit for that circuit than one of the other jugglers assigned to it.

 So if a team is full at some point, the only way for someone to get in is to be a better match
 then the worst juggler on the team. Any replacement improves the match between worst team member 
 and the circuit. So if some team member gets replaced, he won't be able to come back to same team later. 
 Same is true for a juggler who tries to join a team and fails. 
 So given an unassigned juggler, we can try to put him on his most favourite team. If we are not able to do that,
 we'll try to place him on his second favourite team and so on. If some juggler is replaced, we'll try to put him
 on his next favourite team. 

 Notes

 For simplicity of the code, each step is same : trying to assign a juggler from a line to 
 a team on the top of his preference list. Replaced/rejected juggler goes back to the list. 
 List of jugglers is used as a stack so logic is same.
 Once juggler is picked form the line, the top of his preference list is popped and he is never
 considered as a candidate for a same team twice.
 Since it only takes comparison between the worst team member and a candidate to decide,
 we can keep team members in a heap, key would be a match score. 

