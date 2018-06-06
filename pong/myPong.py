import numpy as np 
import tensorflow as tf 
import os.path
import gym

#arg parse and some const definitions

ACTION_UP = 2
ACTION_DOWN = 3
action_dict = {ACTION_DOWN:0, ACTION_UP:1}

HIDDEN_LAYER_SIZE = 200
LEARNING_RATE = 0.0005
EPS_PER_BATCH = 1
DISCOUNT_FACTOR = 0.99
EPS_PER_CHECKPOINT = 10
CKPT_DIR = 'checkpoints'
LOAD = True
RENDER = True 
PRINT_ROUND_INFORMATION = False

OBSER_SIZE = 6400
class Network:
    def __init__(self,hidden_layer_size,learning_rate,checkpoints_dir = CKPT_DIR):
        self.learning_rate = learning_rate
        self.sess = tf.InteractiveSession()
        self.observations = tf.placeholder(tf.float32,[None, OBSER_SIZE])
        self.actions = tf.placeholder(tf.float32,[None,1])
        self.advantage = tf.placeholder(tf.float32,[None,1],name='advantage')

        #caculatation graph definition
        hidden = tf.layers.dense(
            self.observations,
            units=hidden_layer_size,
            activation=tf.nn.relu,
            kernel_initializer=tf.contrib.layers.xavier_initializer())
        
        self.up_prob = tf.layers.dense(
            hidden,
            units = 1,
            activation = tf.sigmoid,
            kernel_initializer=tf.contrib.layers.xavier_initializer()
        )

        self.loss = tf.losses.log_loss(
            labels = self.actions,
            predictions=self.up_prob,
            weights=self.advantage
        )
        optimizer = tf.train.AdamOptimizer(self.learning_rate)
        self.training = optimizer.minimize(self.loss)

        tf.global_variables_initializer().run()

        self.saver = tf.train.Saver()
        self.checkpointFile = os.path.join(checkpoints_dir,'pgn.ckpt')

    def read_ckpt(self):
        print("Loading checkpoint from file...")
        self.saver.restore(self.sess,self.checkpointFile)

    def save_ckpt(self):
        print("Saving checkpoing to file")
        self.saver.save(self.sess, self.checkpointFile)

    def getValue(self, observations):
        up_P = self.sess.run(
            self.up_prob,
            feed_dict = {self.observations: observations.reshape([1,-1])}
        )
        return up_P

    def train(self,sar_tuple):
        print('Training with %d (state, action, reward) tuples' %
              len(sar_tuple))
        states, actions, rewards = zip(*sar_tuple)
        states = np.vstack(states)
        actions = np.vstack(actions)
        rewards = np.vstack(rewards)
        feed_dict = {
            self.observations: states,
            self.actions: actions,
            self.advantage: rewards 
        }
        self.sess.run(self.training, feed_dict)



#from Andrej...
#change picture to an 1-D array of 6400 elements
def preprocessing(pic):
    pic = pic[35:195]       #crop
    pic = pic[::2,::2,0]    #smaller
    pic[pic == 144] = 0     #mask bg
    pic[pic == 109] = 0     #mask bg
    pic[pic != 0] = 1       #identify fg
    return pic.astype(np.float).ravel() #to 1-D

#rewards : come from EPS_PER_BATCH epsiode(s)
#          [[rewards in round 1],[rewards in round 2]...]
def discount_rewards(rewards, discount_f):
    ret = np.zeros_like(rewards)
    for t in range(len(rewards)):
        temp_sum = 0
        discount = 1
        for k in range(t,len(rewards)):
            temp_sum += rewards[k] * discount
            discount *= discount_f
            if rewards[k]!=0:
                break
        ret[t] = temp_sum
    return ret

if __name__ == "__main__":
    env = gym.make('Pong-v0')
    network = Network(HIDDEN_LAYER_SIZE,LEARNING_RATE,)
    if LOAD == True:
        network.read_ckpt()
    
    batch_sar_tuples = []
    smoothed_reward = None
    eps_cnt = 1
    while True:
        print('Staring eps {}'.format(eps_cnt))

        eps_done = False
        eps_reward_sum = 0
        round_cnt = 1

        last_obs = env.reset()
        last_obs = preprocessing(last_obs)
        action = env.action_space.sample()
        obs,_,_,_ = env.step(action)
        obs = preprocessing(obs)
        step_cnt = 1

        while not eps_done:
            if RENDER:
                env.render()
            
            obs_delta = obs - last_obs
            last_obs = obs
            up_prob = network.getValue(obs_delta)
            if np.random.uniform() < up_prob:
                action = ACTION_UP
            else:
                action = ACTION_DOWN

            obs, reward, eps_done, info = env.step(action)
            obs = preprocessing(obs)
            eps_reward_sum += reward
            step_cnt += 1
            tp = (obs_delta,action_dict[action],reward)
            batch_sar_tuples.append(tp)

            if PRINT_ROUND_INFORMATION:
                if reward == -1:
                    print('round {} lost...'.format(round_cnt))
                elif reward == +1:
                    print('round {} win!'.format(round_cnt))
            
            if reward!=-1:
                round_cnt += 1
                step_cnt = 0
            
        print('Eps {} finished! Total {} rounds, rewards:{} discounted rewards:{}'.format(eps_cnt,round_cnt,eps_reward_sum,smoothed_reward))
        computer = 21
        myS = 21
        if  eps_reward_sum < 0:
            myS += eps_reward_sum
        else:
            computer -= eps_reward_sum
        print('Score is {}:{} (computer : my BOT)'.format(computer,myS))

        if smoothed_reward is None:
            smoothed_reward = eps_reward_sum
        else:
            smoothed_reward = smoothed_reward * 0.99 + eps_reward_sum * 0.01

        if eps_cnt % EPS_PER_BATCH == 0:
            states,actions,rewards = zip(*batch_sar_tuples)
            rewards = discount_rewards(rewards,DISCOUNT_FACTOR)
            rewards -= np.mean(rewards)
            rewards /= np.std(rewards)
            batch_sar_tuples = list(zip(states,actions,rewards))
            network.train(batch_sar_tuples)
            batch_sar_tuples = []

        if eps_cnt % EPS_PER_CHECKPOINT == 0:
            network.save_ckpt()

        eps_cnt += 1
        round_cnt = 0
            


            


