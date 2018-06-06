import gym
from gym import wrappers
import numpy as np 
import math
import random
from collections import deque


# constants
GAME = 'CartPole-v1'
EPSILON = 0.1
ALPHA = 0.1
GAMMA = 1.0
MAX_EPSOIDE = 2500
WIN_TICKS = 495


class Agent():
    def __init__(self,env):
        self.env = env
        self.dimension = (1,1,6,12,)
        self.Q = np.zeros(self.dimension + (self.env.action_space.n,))
        self.ada_divisor = 25

    def setenv(self, env):
        self.env = env

    def transfer_state(self, obs):
        upper_bounds = [self.env.observation_space.high[0], 0.5, self.env.observation_space.high[2], math.radians(50)]
        lower_bounds = [self.env.observation_space.low[0], -0.5, self.env.observation_space.low[2], -math.radians(50)]
        ratios = [(obs[i] + abs(lower_bounds[i])) / (upper_bounds[i] - lower_bounds[i]) for i in range(len(obs))]
        new_obs = [int(round((self.dimension[i] - 1) * ratios[i])) for i in range(len(obs))]
        new_obs = [min(self.dimension[i] - 1, max(0, new_obs[i])) for i in range(len(obs))]
        return tuple(new_obs)

    def get_alpha(self, t):
        return max(ALPHA, min(1.0, 1.0 - math.log10((t + 1) / self.ada_divisor)))

    def get_ep(self, t):
        return max(EPSILON, min(1.0, 1.0 - math.log10((t + 1) / self.ada_divisor)))


    def update_Qtable(self,state_old,action,reward,state_new, eps):
        alpha = self.get_alpha(eps)
        self.Q[state_old][action] += alpha * (reward + GAMMA * np.max(self.Q[state_new]) - self.Q[state_old][action])

    #public interfaces 
    def get_action(self,stat,eps):
        state = self.transfer_state(stat)
        e = self.get_ep(eps)
        return self.env.action_space.sample() if (np.random.random() <= e) else np.argmax(self.Q[state])

    def update_info(self, state, action, reward, nextstate, done, eps):
        t_state = self.transfer_state(state)
        t_new_state = self.transfer_state(nextstate)
        self.update_Qtable(t_state,action,reward,t_new_state,eps)

    def print_Q(self):
        print(self.Q)


    

if __name__ == "__main__":
    env = gym.make(GAME)#.unwrapped   
#    env = wrappers.Monitor(env, '/tmp/cartpole-experiment-1',force=True)
    agent = Agent(env)
    scores = deque(maxlen = 100)
    for eps in range(MAX_EPSOIDE):
        state = env.reset()
        agent.setenv(env)
        done = False 
        st = 0
        while not done:
            if eps % 100 == 0:
                env.render()
            action = agent.get_action(state,eps)
            next_st, reward, done, _ = env.step(action)
            agent.update_info(state,action,reward,next_st,done,eps)
            state = next_st
            st += 1
            if done:
                scores.append(st)
                break
        
        if eps % 100 == 0:
            mean_score = np.mean(scores)
            scores.clear()
            #agent.print_Q()
            print('[Episode {}] - Mean survival time over last 100 episodes was {} ticks.'.format(eps, mean_score))
            if mean_score >= WIN_TICKS:
                break
