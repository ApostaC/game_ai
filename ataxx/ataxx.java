import java.util.*;
import org.json.simple.JSONValue;
import java.awt.event.*;
import javax.swing.Timer;
public class Main {
	public static int searchCount=0;
	public static int max(int a,int b)
	{
		return a>b?a:b;
	}
	public static int min(int a,int b)
	{
		return a<b?a:b;
	}
	public static class Table{
		public final static int MIN=-64;
		public final static int MAX=64;
		public final static int BLACK=0;
		public final static int WHITE=1;
		public long[] p;	//0 means black; 1 means white;	
							//white is max ; black is min
		public int player;
		public Table()
		{
			p=new long[2];
			setBlack(0,0);setBlack(6,6);setWhite(6,0);setWhite(0,6);
			player=BLACK;
		}
		public Table(long b,long w,int pl)
		{
			p[BLACK]=b;p[WHITE]=w;
			player=pl;
		}
		public void setBlack(int i,int j)
		{
			p[BLACK]^=1L << (i<<3 | j);
		}
		public void setWhite(int i,int j)
		{
			p[WHITE]^=1L << (i<<3 | j);
		}
		public void procStep(int fr,int to,int color)
		{
			p[color]=p[color] ^ (far[fr] >>> to & 1L) << fr ^ 1L<<to;
			long change=p[color ^ 1] & near[to];
			p[color] |= change; p[color ^ 1] ^= change;
		}
		public void procStep(long fr,long to)
		{
			int color=player;
			p[color]=p[color] ^ (far[(int) fr] >>> to & 1L) << fr ^ 1L<<to;
			long change=p[color ^ 1] & near[(int) to];
			p[color] |= change; p[color ^ 1] ^= change;
		}

		public int eval()
		{
			//TODO: accomplish it about ending... +1???
			if (p[BLACK]==0) return MAX+1;	//black=0 then white win then return max
			if (p[WHITE]==0) return MIN-1;	//white=0 then black win then return min
			int bl=getNumberOne(p[0]);int wh=getNumberOne(p[1]);
			return wh-bl;
		}
		public void changePlayer()
		{
			player=1-player;
		}
		public int getBlank()
		{
			int bl=getNumberOne(p[0]);int wh=getNumberOne(p[1]);
			return 49-bl-wh;
		}
		public int noMove()
		{
			if (player==BLACK)
			{
				if (getNumberOne(p[BLACK])>24) return MIN-1;
				else return MAX+1;
			}
			else 
			{
				if (getNumberOne(p[WHITE])>24) return MAX+1;
				else return MIN-1;
			}
		}
		public Table copy()
		{
			Table t=new Table();
			t.p[0]=this.p[0];
			t.p[1]=this.p[1];
			t.player=this.player;
			return t;
		}
		public String toString()
		{
			return Long.toString(p[0],16)+Long.toString(p[1],16);
		}
	}
	/**
	 * @author lenovo
	 * record the operation
	 */
	public static class Operation implements Comparable<Operation>{
		
		public int fr;
		public int to;
		public int value;
		public Operation()
		{
			fr=0;to=0;value=0;
		}
		public Operation(int f,int t)
		{
			fr=f;to=t;value=0;
		}
		public Operation (int f,int t,int v)
		{
			fr=f;to=t;value=v;
		}
		@Override
		public int compareTo(Operation o) {
			if (o.value==this.value) return 0;
			else if(this.value<o.value) return -1;
			else return 1;
		}
		public boolean equals(Operation o)
		{
			return (o.fr==fr && o.to==to);
		}
		public Operation copy()
		{
			return new Operation(this.fr,this.to,this.value);
		}
	}
	public static class State{
		public Table table;
		public Operation op;
		public State()
		{
			table=new Table();op=new Operation();
		}
		public State(Table t)
		{
			table=t;op=new Operation();
		}
		public State copy()
		{
			State ret = new State(this.table.copy());
			ret.op = new Operation(op.fr,op.to,op.value);
			return ret;
		}
	}
	
	public static long[] near=new long[64];
	public static long[] far=new long[64];
	public static long[] all=new long[64];
	public static final int[] dx={0, 0, 1, -1, -1, 1, -1, 1, 2, 2, 2, 2,
			2, -2, -2, -2, -2, -2, 1, 0, -1, 1, 0, -1};
	public static final int[] dy={1, -1, 0, 0, 1, -1, -1, 1, 2, -2,1, 0,
			-1, 2,  -2, 1,  0,  -1,  2, 2, 2,  -2, -2,-2};
	public static void init()
	{
		for(int i=0;i<7;++i)
		for(int j=0;j<7;++j)
		{
			long temp=0;
			for(int k=0;k<8;++k)
			{
				int xx=i+dx[k];int yy=j+dy[k];
				if (xx>=0 && yy>=0 && xx<7 && yy<7)
					temp ^= 1L << (xx<<3 | yy);
			}near[i<<3 | j]=temp;
			temp=0;
			for(int k=8;k<24;++k)
			{
				int xx=i+dx[k];int yy=j+dy[k];
				if (xx>=0 && yy>=0 && xx<7 && yy<7)
					temp ^= 1L << (xx<<3 | yy);
			}far[i<<3 | j]=temp;
			temp=0;
			for(int k=0;k<24;++k)
			{
				int xx=i+dx[k];int yy=j+dy[k];
				if (xx>=0 && yy>=0 && xx<7 && yy<7)
					temp ^= 1L << (xx<<3 | yy);
			}all[i<<3 | j]=temp;
		}
	}

	/**
	 * @param n
	 * @return the number of 1 in binary term of n
	 */
	public static int getNumberOne(long n)
	{
		return Long.bitCount(n);
	}
	public static int getFirstOne(long n)
	{
		return Long.numberOfTrailingZeros(n);
	}
	public static ArrayList<Operation> generateBranch(Table t,int initD,Operation op)
	{
		ArrayList<Operation> ops=new ArrayList<Operation>();
		long blank=~(t.p[0]|t.p[1]);long now=t.p[t.player];
		int fr=0;int to=0;
		for(long tos=blank;tos!=0;tos^=(1L<<to))
		{
			Table tt=t.copy();
			to=getFirstOne(tos);
			long nearfr=(near[to] & now);
			if (nearfr!=0) 
			{
				fr=getFirstOne(nearfr);
				tt.procStep(fr, to);
				tt.changePlayer();
				Operation tempO=new Operation(fr,to);
				if (!op.equals(tempO))
					ops.add(new Operation(fr,to,miniMaxN(new State(tt),Table.MIN,Table.MAX,initD/2)));
			}
			fr=0;
			for(long farfr=(far[to] & now);farfr!=0;farfr^=(1L<<fr))
			{
				fr=getFirstOne(farfr);
				tt=t.copy();
				tt.procStep(fr, to);
				tt.changePlayer();
				Operation tempO=new Operation(fr,to);
				if (!op.equals(tempO))
					ops.add(new Operation(fr,to,miniMaxN(new State(tt),Table.MIN,Table.MAX,initD/2)));
			}
		}
		if (t.player==Table.BLACK) Collections.sort(ops);
		else Collections.sort(ops, Collections.reverseOrder());
		ops.add(0, op);
		return ops;
	}
	public static Table mainT=new Table();
	public static State ans=new State(mainT.copy());
	
	public static class Answer{
		private long fr;
		private long to;
		public Answer(Operation op)
		{
			fr=op.fr;
			to=op.to;
		}
		public int getX0(){return (int)fr>>>3;}
		public int getY0(){return (int)fr & 7;}
		public int getX1(){return (int)to>>>3;}
		public int getY1(){return (int)to & 7;}
		public String toString()
		{
			return getX0()+" "+getY0()+" "+getX1()+" "+getY1();
		}
	}
	
	public static int miniMaxN(State st,int alpha,int beta,int depth)
	{
		searchCount++;
		if (depth==0) return st.table.eval();
		long flag=0L;
		Table t=st.table;
		long blank=~(t.p[0]|t.p[1]);long now=t.p[t.player];
		int fr=0;int to=0;
		int v=0;
		if (t.player==Table.WHITE) v=Table.MIN;
		else v=Table.MAX;
		for(long tos=blank;tos!=0;tos^=(1L<<to))
		{
			to=getFirstOne(tos);
			long nearfr=(near[to] & now);
			if (nearfr!=0) 
			{
				flag=flag | nearfr;
				fr=getFirstOne(nearfr);
				State newTable=new State(st.table.copy());
				newTable.table.procStep(fr, to);
				newTable.table.changePlayer();
				if (t.player==Table.WHITE)
				{
					v=max(v,miniMaxN(newTable,alpha,beta,depth-1));
					if (v>alpha){
						alpha=v;
						st.op=new Operation(fr,to,v);
					}
					if (beta<alpha) break;
				}
				else
				{
					v=min(v,miniMaxN(newTable,alpha,beta,depth-1));
					if (v<beta){
						beta=v;
						st.op=new Operation(fr,to,v);
					}
					if (beta<alpha) break;
				}
			}
			fr=0;
			for(long farfr=(far[to] & now);farfr!=0;farfr^=(1L<<fr))
			{
				flag=flag | farfr;
				fr=getFirstOne(farfr);
				State newTable=new State(st.table.copy());
				newTable.table.procStep(fr, to);
				newTable.table.changePlayer();
				if (t.player==Table.WHITE)
				{
					v=max(v,miniMaxN(newTable,alpha,beta,depth-1));
					if (v>alpha){
						alpha=v;
						st.op=new Operation(fr,to,v);
					}
					if (beta<alpha) break;
				}
				else
				{
					v=min(v,miniMaxN(newTable,alpha,beta,depth-1));
					if (v<beta){
						beta=v;
						st.op=new Operation(fr,to,v);
					}
					if (beta<alpha) break;
				}
			}
		}
		return (flag==0)?t.noMove():v;
	}
	
	public static int miniMax(State st,int alpha,int beta,int depth)
	{
		searchCount++;
		ArrayList<Operation> ops=generateBranch(st.table,depth,st.op);
		if (ops.size()==0) return st.table.noMove();
		if (depth==0) return st.table.eval();
		if (st.table.player==Table.WHITE)
		{
			int v=Table.MIN;
			for(Operation op : ops)
			{
				State nextSt=new State(st.table.copy());
				nextSt.table.procStep(op.fr, op.to);
				nextSt.table.changePlayer();
				int tempv=miniMaxN(nextSt,alpha,beta,depth-1);
				op.value=tempv;
				v=max(v,tempv);
				if (v>alpha)
				{
					alpha=v;
					st.op=op;
				}
				else if (tempv==alpha)
				{
					if (new Random().nextBoolean())
					{
						st.op=op;
					}
				}
				if (alpha>beta) break;
			}
			return v;
		}
		else
		{
			int v=Table.MAX;
			for(Operation op:ops)
			{
				State nextSt=new State(st.table.copy());
				nextSt.table.procStep(op.fr, op.to);
				nextSt.table.changePlayer();
				int tempv=miniMaxN(nextSt,alpha,beta,depth-1);
				op.value=tempv;
				v=min(v,tempv);
				if(v<beta)
				{
					beta=v;
					st.op=op;
				}
				else if (tempv==beta)
				{
					if (new Random().nextBoolean())
					{
						st.op=op;
					}
				}
				if(alpha>beta) break;
			}
			return v;
		}
	}
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		init();

		int stopTime=2500;
		ActionListener listener=new ActionListener(){
			public void actionPerformed(ActionEvent e)
			{
				printToZone();
				System.exit(0);
			}
		};
		
		Timer tm=new Timer(stopTime,listener);
		tm.start();
		
		Scanner sc=new Scanner(System.in);
		String input = sc.nextLine();
		Map<String,List<Map<String,Long> > > inputJSON=(Map<String,List<Map<String,Long> > >) JSONValue.parse(input);
		final int turnID=inputJSON.get("responses").size();
		List<Map<String,Long> > requests=inputJSON.get("requests");
		List<Map<String,Long> > responses=inputJSON.get("responses");
		Map<String,Map<String,Long>> input2=(Map<String,Map<String,Long>>) JSONValue.parse(input);
		int currBotColor=requests.get(0).get("x0")<0?0:1;
		long x0=0,y0=0,x1=0,y1=0;
		for(int i=0;i<turnID;i++)
		{
			x0=(long)requests.get(i).get("x0");
			y0=(long)requests.get(i).get("y0");
			x1=(long)requests.get(i).get("x1");
			y1=(long)requests.get(i).get("y1");
			if (x0>=0)
			{
				long fr=(x0<<3 | y0);long to=(x1<<3 | y1);
				mainT.player=1-currBotColor;
				mainT.procStep(fr, to);
			}
			x0=(long)responses.get(i).get("x0");
			y0=(long)responses.get(i).get("y0");
			x1=(long)responses.get(i).get("x1");
			y1=(long)responses.get(i).get("y1");
			if (x0>=0)
			{
				long fr=(x0<<3 | y0);long to=(x1<<3 | y1);
				mainT.player=currBotColor;
				mainT.procStep(fr, to);
				
			}
		}
		x0=(long)requests.get(turnID).get("x0");
		y0=(long)requests.get(turnID).get("y0");
		x1=(long)requests.get(turnID).get("x1");
		y1=(long)requests.get(turnID).get("y1");
		if (x0>=0)
		{
			long fr=(x0<<3 | y0);long to=(x1<<3 | y1);
			mainT.player=1-currBotColor;
			mainT.procStep(fr, to);
			
		}
		mainT.player=currBotColor;
		ans.table=mainT.copy();
		Thread t=new Thread(new Runnable(){
			public void run()
			{
				miniMaxN(ans,Table.MIN,Table.MAX,1);
				for(int depth=2;;depth++)
				{
					State tempState = ans.copy();
					miniMax(tempState,Table.MIN,Table.MAX,depth);
					ans = tempState.copy();
					//System.out.println(depth+" "+searchCount+" "+ans.op.value);
				}	
			}
		});
		t.start();
		try {
			Thread.sleep(stopTime-50);
			t.interrupt();//tt.interrupt();
			Thread.sleep(150);
		} catch (InterruptedException e1) {
			printToZone();
		}
		printToZone();
		sc.close();
	}
	public static void printToZone()
	{
		Map<String,Map<String ,Long>> outputJSON=new HashMap<String, Map<String,Long>>();
		Map<String,Long> oo=new HashMap<String, Long>();
		Answer an=new Answer(ans.op);
		int x0=an.getX0();int y0=an.getY0();
		int x1=an.getX1();int y1=an.getY1();
		oo.put("x0", (long) x0);oo.put("y0", (long) y0);oo.put("x1", (long) x1);
		oo.put("y1",(long) y1);
		outputJSON.put("response", oo);
		Map<String,Long> oo2=new HashMap<String, Long>();
		oo2.put("searchCount", (long) searchCount);
		outputJSON.put("debug", oo2);
		mainT.procStep(ans.op.fr, ans.op.to);
		Map<String,Long> oo3=new HashMap<String, Long>();
		oo3.put("black", mainT.p[Table.BLACK]);
		oo3.put("white", mainT.p[Table.WHITE]);
		outputJSON.put("globaldata", oo3);
		System.out.println(JSONValue.toJSONString(outputJSON));

	}

}
