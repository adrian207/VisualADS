package cnd.conflict.frame;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.MenuBar;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashSet;
import java.util.Set;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.border.Border;
import javax.swing.tree.DefaultMutableTreeNode;

import cnd.conflict.dao.PolicyDAO;
import cnd.conflict.service.Service;
import cnd.conflict.util.DataBaseConn;

class HideWizard extends Thread {
	
	public Frame frame;
	 
    public HideWizard() {
 
    }
    
    public HideWizard(Frame frame) {
    	this.frame = frame;
    }
    
    private boolean isFileExist(String filePath) {
		File f = new File(filePath);
		if (f.exists()) {
			return true;
		} else {
			return false;
		}
	}
    
    public String initPolicyDir() {
		String curDir = null;
		String filePath = System.getProperty("user.dir");
		if (isFileExist(filePath + "/resources")) {
			curDir = filePath + "/resources/";
		} else {
			// ���JAR������·��
			filePath = new Cndpcd().getClass().getProtectionDomain()
					.getCodeSource().getLocation().getFile();
			try {
				filePath = java.net.URLDecoder.decode(filePath, "UTF-8");
			} catch (java.io.UnsupportedEncodingException e) {
				e.printStackTrace();
			}
			java.io.File jarFile = new java.io.File(filePath);
			java.io.File parent = jarFile.getParentFile();
			java.io.File grandPa = parent.getParentFile();
			if (grandPa != null) {
				filePath = grandPa.getAbsolutePath();
				try {
					filePath = java.net.URLDecoder.decode(filePath, "UTF-8");
				} catch (java.io.UnsupportedEncodingException e) {
					e.printStackTrace();
				}
			}

			if (isFileExist(filePath + "/policy")) {
				curDir = filePath + "/policy/";
			} else {
				JOptionPane.showMessageDialog(null, "initPolicyDir Error",
						"Error", JOptionPane.ERROR_MESSAGE);
			}
		}
		return curDir;
	}
 
    public void run() {
    	while (true) {
    		String PFilePath = initPolicyDir() + "needShow.txt";
    		FileInputStream fis = null;
    		InputStreamReader isr = null;
    		BufferedReader br = null;
    		String str = "";
    		
			try {
				fis = new FileInputStream(PFilePath);
				isr = new InputStreamReader(fis);// InputStreamReader
				// ���ֽ���ͨ���ַ���������,
				br = new BufferedReader(isr);// ���ַ��������ж�ȡ�ļ��е�����,��װ��һ��new
				// InputStreamReader�Ķ���

				while ((str = br.readLine()) != null) {
					if (str.equals("needShow=1")) {
						frame.setVisible(true);
						return;
					}
				}
			} catch (FileNotFoundException e) {
				//System.out.println("�Ҳ���ָ���ļ�");
				//JOptionPane.showMessageDialog(null, "File not Found Error",
				//		"Error", JOptionPane.ERROR_MESSAGE);
				frame.setVisible(true);
				return;
			} catch (IOException e) {
				System.out.println("��ȡ�ļ�ʧ��");
				JOptionPane.showMessageDialog(null, "File read Error",
						"Error", JOptionPane.ERROR_MESSAGE);
			} finally {
				try {
					if (br != null) br.close();
					if (isr != null) isr.close();
					if (fis != null) fis.close();
					// �رյ�ʱ����ð����Ⱥ�˳��ر���󿪵��ȹر������ȹ�s,�ٹ�n,����m
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			
    	}
    }
 
    public static void main(String[] args) {
    	
    }
}

/**
 * ���������
 * 
 */
public class Frame extends JFrame {

	//private java.util.ResourceBundle resourceBundle;
	private int chineseOrEnglish = 1;
	private HideWizard hideWizard;
	
	private int width = 800;
	private int height = 600;
	private int verticalHeight = 15;
	private int horizontalWidth = 15;
	private JTextArea messageArea;
	private JTable table;
	private JTree tree = createTree();
	

	public JTextArea getMessageArea() {
		return messageArea;
	}
 
	private JScrollPane scrollPaneTree;
	private JScrollPane scrollPaneMessage;
	private JScrollPane scrollPaneTable;
	private Box boxV, boxH, boxHTable;

	// JLabel label = new JLabel();
	// JTextPane textPane = new JTextPane();
	private Container con = getContentPane();

	private Service service = Service.getServiceInstance(this);
	 
	private DataBaseConn dataBaseConn =  DataBaseConn.getDataBaseConnInstance();
	
	private PolicyDAO policyDAO = new PolicyDAO();

	// �Ӹ��˵����¼�������
	ActionListener menuListener = new ActionListener() {
		public void actionPerformed(ActionEvent e) {
			service.menuDo(Frame.this, e.getActionCommand());
		}
	};

	/**
	 * ������
	 */
	public Frame() {
		super();
		// ��ʼ�����JFrame
		init();
		
		setVisible(false);
		hideWizard = new HideWizard(this);
    	hideWizard.start();
	}

	/**
	 * ��ʼ��
	 * 
	 * @return void
	 */
	public void init() {
		// ���ñ���
		this.setTitle("���������������Գ�ͻ��⹤��");
		// ���ô�С
		this.setPreferredSize(new Dimension(width, height));
		// �����˵�
		createMenuBar();
		// ����������
		
		// �������ʻ��ַ�����Դ
		//resourceBundle = java.util.ResourceBundle.getBundle("langue/softwareResources", java.util.Locale.CHINESE);
		// ��������ģʽΪ����
		setLanguage(1);

		// createTree();
		messageArea = createMessage();
		// createTable();

		// ����
		boxH = Box.createHorizontalBox();
		boxV = Box.createVerticalBox();
		boxHTable = Box.createHorizontalBox();

		// scrollPaneTree = new JScrollPane(createTreeTest());
		scrollPaneTree = new JScrollPane(createTree());
		Border etched = BorderFactory.createEtchedBorder();
		scrollPaneTree.setBorder(BorderFactory.createTitledBorder(etched,
				"Policy Tree"));
		boxH.add(Box.createHorizontalStrut(horizontalWidth));
		boxH.add(scrollPaneTree);
		boxH.add(Box.createHorizontalStrut(horizontalWidth));
		scrollPaneMessage = new JScrollPane(messageArea);
		scrollPaneMessage.setBorder(BorderFactory.createTitledBorder(etched,
				"Messages"));
		boxH.add(scrollPaneMessage);
		boxH.add(Box.createHorizontalStrut(horizontalWidth));

		scrollPaneTable = new JScrollPane(createTable());
		scrollPaneTable.setBorder(BorderFactory.createTitledBorder(etched,
				"Policy List"));
		boxHTable.add(Box.createHorizontalStrut(horizontalWidth));
		boxHTable.add(scrollPaneTable);
		boxHTable.add(Box.createHorizontalStrut(horizontalWidth));

		boxV.add(Box.createVerticalStrut(verticalHeight));
		boxV.add(boxH);
		boxV.add(Box.createVerticalStrut(verticalHeight));
		boxV.add(boxHTable);
		boxV.add(Box.createVerticalStrut(verticalHeight));

		this.add(boxV);
		setUndecorated(true);
		this.setVisible(true);
		validate();
		this.pack();
	}

	/**
	 * �����˵���
	 * 
	 * @return void
	 */
	public void createMenuBar() {
		// ����һ��JMenuBar���ò˵�
		JMenuBar menuBar = new JMenuBar();

		// �˵��������飬�������menuItemArrһһ��Ӧ
		// String[] menuArr = { "�ļ�(F)", "����(T)", "�����ʾ(R)","����(H)" };
		String[] menuArr = { "�ļ�", "�л�����������","����Ԥ����", "���彨ģ", "��ͻ���", "�������", "�л�����", "����" };
		// �˵�����������
		String[][] menuItemArr = {
				{ "��(O)", "-", "����(S)", "-", "�༭(E)", "-", "�˳�(X)" },
				{"�����������ݿ�","-", "����ļ����ʿ��Ʋ���"},
				{ "���Խ���", "-", "����ӳ��" , "-", "�����б�"},
				{ "���嵼��", "-", "ʵ������", "-", "��ϵͼ����" },
				{ "��ͻ���", "-", "���ɱ���" }, { "���������" }, { "Ӣ��", "-", "����" },
				{ "����", "-", "����" } };

		// String[][] menuItemArr = { { "Open(O)", "-", "Exit(X)" },
		// { "�Ŵ�(M)", "��С(O)", "-", "��һ��(X)", "��һ��(P)" }, {"�����ʾ"},{ "��������",
		// "����" } };
		// ����menuArr��menuItemArrȥ�����˵�
		for (int i = 0; i < menuArr.length; i++) {
			// �½�һ��JMenu�˵�
			JMenu menu = new JMenu(menuArr[i]);
			for (int j = 0; j < menuItemArr[i].length; j++) {
				// ���menuItemArr[i][j]����"-"
				if (menuItemArr[i][j].equals("-")) {
					// ���ò˵��ָ�
					menu.addSeparator();
				} else {
					// �½�һ��JMenuItem�˵���
					JMenuItem menuItem = new JMenuItem(menuItemArr[i][j]);
					menuItem.addActionListener(menuListener);
					// �Ѳ˵���ӵ�JMenu�˵�����
					menu.add(menuItem);
				}
			}
			// �Ѳ˵��ӵ�JMenuBar��
			menuBar.add(menu);
		}
		// ����JMenubar
		this.setJMenuBar(menuBar);
	}
	
	public void updateMenuBarToEnglish() {
		setLanguage(0);
		JMenuBar menuBar = this.getJMenuBar();
		menuBar.removeAll();
		
		// �˵��������飬�������menuItemArrһһ��Ӧ
		// String[] menuArr = { "�ļ�(F)", "����(T)", "�����ʾ(R)","����(H)" };
		String[] menuArr = { "File", "Mode","Preprocess", "Semantic-Model", "Detect", "Result", "Language", "Help" };
		// �˵�����������
		String[][] menuItemArr = {
				{ "Open(O)", "-", "Save(S)", "-", "Edit(E)", "-", "Exit(X)" },
				{"Connect Sample Database","-", "Detect File Policies"},
				{ "Parse Policies", "-", "Semantic Mapping" , "-", "Policy List"},
				{ "Import Ontology", "-", "Load Instance", "-", "Generate Relation Graph" },
				{ "Detect Conflicts", "-", "Generate Report" }, { "Analyze Result" }, { "English", "-", "Chinese" }, 
				{ "Help", "-", "About" } };
		
		// ����menuArr��menuItemArrȥ�����˵�
		for (int i = 0; i < menuArr.length; i++) {
			// �½�һ��JMenu�˵�
			JMenu menu = new JMenu(menuArr[i]);
			for (int j = 0; j < menuItemArr[i].length; j++) {
				// ���menuItemArr[i][j]����"-"
				if (menuItemArr[i][j].equals("-")) {
					// ���ò˵��ָ�
					menu.addSeparator();
				} else {
					// �½�һ��JMenuItem�˵���
					JMenuItem menuItem = new JMenuItem(menuItemArr[i][j]);
					menuItem.addActionListener(menuListener);
					// �Ѳ˵���ӵ�JMenu�˵�����
					menu.add(menuItem);
				}
			}
			// �Ѳ˵��ӵ�JMenuBar��
			menuBar.add(menu);
		}
		// ����JMenubar
		this.setJMenuBar(menuBar);
	}
	
	public void updateMenuBarToChinese() {
		setLanguage(1);
		JMenuBar menuBar = this.getJMenuBar();
		menuBar.removeAll();
		
		// �˵��������飬�������menuItemArrһһ��Ӧ
		// String[] menuArr = { "�ļ�(F)", "����(T)", "�����ʾ(R)","����(H)" };
		String[] menuArr = { "�ļ�", "�л�����������","����Ԥ����", "���彨ģ", "��ͻ���", "�������", "�л�����", "����" };
		// �˵�����������
		String[][] menuItemArr = {
				{ "��(O)", "-", "����(S)", "-", "�༭(E)", "-", "�˳�(X)" },
				{"�����������ݿ�","-", "����ļ����ʿ��Ʋ���"},
				{ "���Խ���", "-", "����ӳ��" , "-", "�����б�"},
				{ "���嵼��", "-", "ʵ������", "-", "��ϵͼ����" },
				{ "��ͻ���", "-", "���ɱ���" }, { "���������" }, { "Ӣ��", "-", "����" },
				{ "����", "-", "����" } };
		
		// ����menuArr��menuItemArrȥ�����˵�
		for (int i = 0; i < menuArr.length; i++) {
			// �½�һ��JMenu�˵�
			JMenu menu = new JMenu(menuArr[i]);
			for (int j = 0; j < menuItemArr[i].length; j++) {
				// ���menuItemArr[i][j]����"-"
				if (menuItemArr[i][j].equals("-")) {
					// ���ò˵��ָ�
					menu.addSeparator();
				} else {
					// �½�һ��JMenuItem�˵���
					JMenuItem menuItem = new JMenuItem(menuItemArr[i][j]);
					menuItem.addActionListener(menuListener);
					// �Ѳ˵���ӵ�JMenu�˵�����
					menu.add(menuItem);
				}
			}
			// �Ѳ˵��ӵ�JMenuBar��
			menuBar.add(menu);
		}
		// ����JMenubar
		this.setJMenuBar(menuBar);
	}

	private JTree createTree() {
		DefaultMutableTreeNode root = new DefaultMutableTreeNode("CNDPOLICY");
		tree = new JTree(root);
		return tree;
	}

	private JTextArea createMessage() {
		JTextArea messageArea = new JTextArea(6, 20);
		return messageArea;
	}

	public JTable createTable() {

		Object a[][];
		Object name[] = { "PolicyID", "Organization", "Context", "Role",
				"View", "Activity", "Measure" };
		// ��д���е�����

		int policyAmount = policyDAO.getPolicyAmount();

		a = new Object[policyAmount][name.length];

		table = new JTable(a, name);
		return table;
	}

	public JTable getJTable() {
		return this.table;
	}

	public JTree getJTree() {
		return this.tree;
	}

	public int getLanguage() {
		return chineseOrEnglish;
	}
	
	public void setLanguage(int chineseOrEnglish) {
		this.chineseOrEnglish = chineseOrEnglish;
	}
}