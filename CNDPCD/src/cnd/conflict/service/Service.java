package cnd.conflict.service;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.StringTokenizer;

import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.SwingConstants;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumn;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;

import cnd.conflict.dao.PolicyDAO;
import cnd.conflict.detect.ConflictAnalysis;
import cnd.conflict.detect.RelationGraph;
import cnd.conflict.detect.Report;
import cnd.conflict.entity.CndPolicy;
import cnd.conflict.frame.Cndpcd;
import cnd.conflict.frame.Frame;
import cnd.conflict.frame.PolicyEditor;
import cnd.conflict.frame.ResultAnalysisFrame;
import cnd.conflict.util.About;
import cnd.conflict.util.DataBaseConn;
import cnd.conflict.util.Help;
import cnd.conflict.util.ResultBarChart;

/**
 * ҵ����
 * 
 * @author YC LYZ
 */

public class Service {
	private static Service service = null;
	private static Frame frame;

	private DataBaseConn dataBaseConn = DataBaseConn.getDataBaseConnInstance();

	// JFileChooser
	private JFileChooser filechooser = new JFileChooser();

	private RelationGraph relationGraph = RelationGraph.getInstance(frame);

	private ConflictAnalysis conflictAnalysis = ConflictAnalysis.getInstance();

	PolicyDAO policyDAO = new PolicyDAO();
	private PolicyEditor policyEditor;

	private Report notepadSample = Report.getInstance();

	private List<String> contentArr = new ArrayList<String>();

	// ������Ҫ�����������Ϣ
	private String[] domainNames = new String[20];

	
	public static Frame getFrame() {
		return frame;
	}

	public PolicyDAO getPolicyDAO() {
		return policyDAO;
	}

	public String[] getDomainNames() {
		return domainNames;
	}

	/**
	 * ˽�й�����
	 */
	protected Service(Frame frame) {
		Service.frame = frame;
	}

	/**
	 * ��ȡ��̬ʵ��
	 * 
	 * @return Service
	 */
	public static Service getServiceInstance(Frame frame) {
		Service.frame = frame;
		if (service == null) {
			service = new Service(frame);
		}

		return service;
	}

	/**
	 * ���ļ�
	 * 
	 * @param frame
	 * 
	 * @return void
	 */
	public void open(Frame frame) {
		// ���ѡ���
		int i = filechooser.showOpenDialog(frame); // ��ʾ���ļ��Ի���

		if (i == JFileChooser.APPROVE_OPTION) { // ����Ի����д�ѡ��
			File f = filechooser.getSelectedFile(); // �õ�ѡ����ļ�
			try {
				BufferedReader br = new BufferedReader(new FileReader(f));
				String str;

				StringBuffer strBuffer = new StringBuffer();
				while ((str = br.readLine()) != null) {
					contentArr.add(str);
					strBuffer.append(str + "\r\n");
				}

				frame.getMessageArea().setText(strBuffer.toString());

				if (br != null) {
					br.close();
				}
			} catch (Exception ex) {
				ex.printStackTrace(); // ���������Ϣ
			}
		}

	}

	// ���Խ���ʱ���ļ�������Ϣ�������ݿ�
	public void policyAnalysis(Frame frame) {
		String analysisInfo;
		if ("fileservices" == dataBaseConn.getUrl_database()) {
			//�����ļ���ȡ�߳�
			new FileReadThread(this).start();
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

	private boolean isFileExist(String filePath) {
		File f = new File(filePath);
		if (f.exists()) {
			return true;
		} else {
			return false;
		}
	}

	public void initJTable(Frame frame) {
		JTable table = frame.getJTable();
		Object a[][];
		Object name[] = { "PolicyID", "Organization", "Context", "Role",
				"View", "Activity", "Measure" };
		int policyAmount = policyDAO.getPolicyAmount();

		a = new Object[policyAmount][name.length];
		for (int i = 0; i < policyAmount; i++) {
			for (int j = 0; j < name.length; j++) {
				CndPolicy cndPolicy = policyDAO.getOnePolicy(i + 1);
				if (j == 0) {
					a[i][j] = cndPolicy.getPolicyId();
				}
				if (j == 1) {
					a[i][j] = cndPolicy.getOrganization();
				}
				if (j == 2) {
					a[i][j] = cndPolicy.getContext();
				}
				if (j == 3) {
					a[i][j] = cndPolicy.getRole();
				}
				if (j == 4) {
					a[i][j] = cndPolicy.getView();
				}
				if (j == 5) {
					a[i][j] = cndPolicy.getActivity();
				}
				if (j == 6) {
					a[i][j] = cndPolicy.getMeasure();
				}
			}
		}
		DefaultTableModel model = new DefaultTableModel(a, name);
		table.setModel(model);

		table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

		FitTableColumns(table);

		DefaultTableCellRenderer r = new DefaultTableCellRenderer();
		r.setHorizontalAlignment(JLabel.CENTER);
		table.setDefaultRenderer(Object.class, r);
	}

	public void FitTableColumns(JTable myTable) {
		JTableHeader header = myTable.getTableHeader();
		int rowCount = myTable.getRowCount();
		Enumeration columns = myTable.getColumnModel().getColumns();
		while (columns.hasMoreElements()) {
			TableColumn column = (TableColumn) columns.nextElement();
			int col = header.getColumnModel().getColumnIndex(
					column.getIdentifier());
			int width = (int) myTable.getTableHeader().getDefaultRenderer()
					.getTableCellRendererComponent(myTable,
							column.getIdentifier(), false, false, -1, col)
					.getPreferredSize().getWidth();
			for (int row = 0; row < rowCount; row++) {
				int preferedWidth = (int) myTable.getCellRenderer(row, col)
						.getTableCellRendererComponent(myTable,
								myTable.getValueAt(row, col), false, false,
								row, col).getPreferredSize().getWidth();
				width = Math.max(width, preferedWidth);
			}
			header.setResizingColumn(column); // ���к���Ҫ
			// +10����һ����϶
			column.setWidth(width + myTable.getIntercellSpacing().width + 10);
		}
	}

	public void initJTree(Frame frame) {
		JTree tree = frame.getJTree();
		DefaultMutableTreeNode root = new DefaultMutableTreeNode("CNDPOLICY");
		DefaultMutableTreeNode t1 = new DefaultMutableTreeNode("Organization");
		DefaultMutableTreeNode t1_1 = new DefaultMutableTreeNode("network");
		DefaultMutableTreeNode t2 = new DefaultMutableTreeNode("Context");

		DefaultMutableTreeNode t3 = new DefaultMutableTreeNode("Role");

		DefaultMutableTreeNode t4 = new DefaultMutableTreeNode("View");

		DefaultMutableTreeNode t5 = new DefaultMutableTreeNode("Activity");

		DefaultMutableTreeNode t6 = new DefaultMutableTreeNode("Type");

		DefaultMutableTreeNode t6_1 = new DefaultMutableTreeNode("Protect");
		DefaultMutableTreeNode t6_2 = new DefaultMutableTreeNode("Detect");
		DefaultMutableTreeNode t6_3 = new DefaultMutableTreeNode("Response");

		DefaultMutableTreeNode t7 = new DefaultMutableTreeNode("Measure");

		root.add(t1);
		t1.add(t1_1);
		root.add(t2);
		root.add(t3);
		root.add(t4);
		root.add(t5);
		root.add(t6);
		t6.add(t6_1);
		t6.add(t6_2);
		t6.add(t6_3);
		root.add(t7);
		DefaultTreeModel model = new DefaultTreeModel(root);
		tree.setModel(model);
		try {
			BufferedReader br = new BufferedReader(new FileReader(new File(
					"K:/javatest/CNDPCD-20130314/resources/CNDPolicy.owl")));
			// akisn0w
			/*
			 * BufferedReader br = new BufferedReader(new FileReader(new File(
			 * "C:/Users/Administrator/Desktop/VisualADS/CNDPCD/resources/CNDPolicy.owl"
			 * )));
			 */
			String str = null;
			StringBuffer strBuffer = new StringBuffer();
			try {
				while ((str = br.readLine()) != null) {
					strBuffer.append(str);
					strBuffer.append("\n");
				}
				frame.getMessageArea().setText(strBuffer.toString());
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		} catch (FileNotFoundException e) {

			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	public void buildJTree(Frame frame) {
		JTree tree = frame.getJTree();
		int policyAmount = policyDAO.getPolicyAmount();
		HashMap<Integer, String> eMap = null;
		DefaultMutableTreeNode root = new DefaultMutableTreeNode("CNDPOLICY");
		DefaultMutableTreeNode t1 = new DefaultMutableTreeNode("Organization");

		DefaultMutableTreeNode t6_1 = null;
		DefaultMutableTreeNode t6_2 = null;
		DefaultMutableTreeNode t6_3 = null;

		DefaultMutableTreeNode t6 = new DefaultMutableTreeNode("Type");

		if ("fileservices" == dataBaseConn.getUrl_database()) {
			DefaultMutableTreeNode[] t1_1 = new DefaultMutableTreeNode[this.domainNames.length];
			for (int k = 0; k < this.domainNames.length; k++) {
				if (null != this.domainNames[k]) {
					t1_1[k] = new DefaultMutableTreeNode(this.domainNames[k]);
					t1.add(t1_1[k]);
				}
			}
			// t1_1 = new DefaultMutableTreeNode("");
			t6_1 = new DefaultMutableTreeNode("Protect");
		} else {
			DefaultMutableTreeNode t1_1 = new DefaultMutableTreeNode("network");
			t6_1 = new DefaultMutableTreeNode("Protect");
			t6_2 = new DefaultMutableTreeNode("Detect");
			t6_3 = new DefaultMutableTreeNode("Response");
			t1.add(t1_1);
			t6.add(t6_2);
			t6.add(t6_3);
		}
		DefaultMutableTreeNode t2 = new DefaultMutableTreeNode("Context");

		eMap = policyDAO.getElementMap(3);
		for (int i = 1; i <= policyAmount; i++) {
			String str = "policy" + i + "_" + eMap.get(i);
			DefaultMutableTreeNode t2_i = new DefaultMutableTreeNode(str);
			t2.add(t2_i);
		}
		DefaultMutableTreeNode t3 = new DefaultMutableTreeNode("Role");
		eMap = policyDAO.getElementMap(4);
		for (int i = 1; i <= policyAmount; i++) {
			String str = "policy" + i + "_" + eMap.get(i);
			DefaultMutableTreeNode t3_i = new DefaultMutableTreeNode(str);
			t3.add(t3_i);
		}
		DefaultMutableTreeNode t4 = new DefaultMutableTreeNode("View");
		eMap = policyDAO.getElementMap(5);
		for (int i = 1; i <= policyAmount; i++) {
			String str = "policy" + i + "_" + eMap.get(i);
			DefaultMutableTreeNode t4_i = new DefaultMutableTreeNode(str);
			t4.add(t4_i);
		}
		DefaultMutableTreeNode t5 = new DefaultMutableTreeNode("Activity");
		eMap = policyDAO.getElementMap(6);
		for (int i = 1; i <= policyAmount; i++) {
			String str = "policy" + i + "_" + eMap.get(i);
			DefaultMutableTreeNode t5_i = new DefaultMutableTreeNode(str);
			t5.add(t5_i);
		}

		HashMap<Integer, Integer> tMap = new HashMap<Integer, Integer>();
		tMap = policyDAO.getTypeMap(2);
		for (int i = 1; i <= policyAmount; i++) {
			if (tMap.get(i) == 0) {
				DefaultMutableTreeNode t6__ = new DefaultMutableTreeNode(
						"policy" + i);
				t6_1.add(t6__);
			}
			if (tMap.get(i) == 1) {
				DefaultMutableTreeNode t6__ = new DefaultMutableTreeNode(
						"policy" + i);
				t6_2.add(t6__);
			}
			if (tMap.get(i) == 2) {
				DefaultMutableTreeNode t6__ = new DefaultMutableTreeNode(
						"policy" + i);
				t6_3.add(t6__);
			}
		}

		DefaultMutableTreeNode t7 = new DefaultMutableTreeNode("Measure");
		eMap = policyDAO.getElementMap(7);
		for (int i = 1; i <= policyAmount; i++) {
			String str = "policy" + i + "_" + eMap.get(i);
			DefaultMutableTreeNode t7_i = new DefaultMutableTreeNode(str);
			t7.add(t7_i);
		}
		root.add(t1);

		root.add(t2);
		root.add(t3);
		root.add(t4);
		root.add(t5);
		root.add(t6);
		t6.add(t6_1);

		root.add(t7);

		DefaultTreeModel model = new DefaultTreeModel(root);
		tree.setModel(model);
		frame.getMessageArea().setText("policy load success!");
	}

	/**
	 * 
	 * @param frame
	 * @return
	 */
	public void save(Frame frame) {
		int i = filechooser.showSaveDialog(frame); // ��ʾ�����ļ��Ի���
		if (i == JFileChooser.APPROVE_OPTION) { // ����Ի����б��水ť
			File f = filechooser.getSelectedFile(); // �õ�ѡ����ļ�
			try {
				FileOutputStream out = new FileOutputStream(f); // �õ��ļ������
				// out.write(frame.getTextPane().getText().getBytes()); //д���ļ�
			} catch (Exception ex) {
				ex.printStackTrace(); // ���������Ϣ
			}
		}
	}

	public void report(Frame frame) {
		try {
			notepadSample.init(frame);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void help(Frame frame) {
		try {
			new Help(frame);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void about(Frame frame) {
		try {
			new About(frame);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void semanticMap(Frame frame) {

		StringBuffer strBuffer = new StringBuffer();

		strBuffer.append("Role Semantic Map:");
		strBuffer.append("\n");
		for (String element : policyDAO.getSemanticRoleMap()) {
			strBuffer.append(element);
			strBuffer.append("\n");
		}
		strBuffer.append("View Semantic Map:");
		strBuffer.append("\n");
		for (String element : policyDAO.getSemanticViewMap()) {
			strBuffer.append(element);
			strBuffer.append("\n");
		}
		strBuffer.append("Activity Semantic Map:");
		strBuffer.append("\n");
		for (String element : policyDAO.getSemanticActivityMap()) {
			strBuffer.append(element);
			strBuffer.append("\n");
		}

		frame.getMessageArea().setText(strBuffer.toString());

	}

	private void result(Frame frame) {
		// TODO Auto-generated method stub
		new ResultBarChart();
		new ResultAnalysisFrame(frame);

	}

	/*
	 * public void test(Frame frame){ frame.test(); }
	 */

	/**
	 * ��Ӧ�˵��Ķ���
	 * 
	 * @param frame
	 * 
	 * @param cmd
	 * 
	 * @return void
	 */
	public void menuDo(Frame frame, String cmd) {
		// ��
		if (cmd.equals("��(O)") || cmd.equals("Open(O)")) {
			open(frame);
		}
		// ����
		if (cmd.equals("����(S)") || cmd.equals("Save(S)")) {
			save(frame);
		}
		// �༭
		if (cmd.equals("�༭(E)") || cmd.equals("Edit(E)")) {

			if (policyEditor == null) {
				policyEditor = PolicyEditor.getInstance();
				policyEditor.init();
			} else {
				policyEditor.setVisible(true);
			}
			/*
			 * policyEditor = PolicyEditor.getInstance(); policyEditor.init();
			 */

		}
		// �����������ݿ�
		if (cmd.equals("�����������ݿ�") || cmd.equals("Connect Sample Database")) {
			dataBaseConn.closeConn();
			dataBaseConn.setUrl_database("policy");
			if (cmd.equals("Connect Sample Database")) {
				frame.getMessageArea().setText("Sample database connected");
			} else {
				frame.getMessageArea().setText("�����ӵ��������ݿ�");
			}
			System.out.println(dataBaseConn.getUrl_database());
		}
		if (cmd.equals("����ļ����ʿ��Ʋ���") || cmd.equals("Detect File Policies")) {
			dataBaseConn.closeConn();
			dataBaseConn.setUrl_database("fileservices");
			// ������ݿ��б������
			policyDAO.deleteAllTable();
			if (cmd.equals("Detect File Policies")) {
				frame.getMessageArea()
						.setText("FileService database connected");
			} else {
				frame.getMessageArea().setText("�����ӵ��ļ��������ݿ�");
			}
			System.out.println(dataBaseConn.getUrl_database());
		}
		// ���Խ���
		if (cmd.equals("���Խ���") || cmd.equals("Parse Policies")) {
			policyAnalysis(frame);
		}
		// �����б�
		if (cmd.equals("�����б�") || cmd.equals("Policy List")) {
			initJTable(frame);
		}
		// ����ӳ��
		if (cmd.equals("����ӳ��") || cmd.equals("Semantic Mapping")) {
			// TODO
			semanticMap(frame);
		}
		// ��ʼ���嵼��
		if (cmd.equals("���嵼��") || cmd.equals("Import Ontology")) {
			initJTree(frame);
		}
		// ʵ������
		if (cmd.equals("ʵ������") || cmd.equals("Load Instance")) {
			buildJTree(frame);
		}
		// ��ϵͼ����
		if (cmd.equals("��ϵͼ����") || cmd.equals("Generate Relation Graph")) {
			relationGraph.init(frame);
		}
		// ��ͻ���
		if (cmd.equals("��ͻ���") || cmd.equals("Detect Conflicts")) {
			if (frame == null || frame.getLanguage() == 1) {
				if ("fileservices" == dataBaseConn.getUrl_database()) {
					conflictAnalysis.conflict_Analysis(frame, "fileservices");
				} else {
					conflictAnalysis.conflict_Analysis(frame);
				}
			} else {
				if ("fileservices" == dataBaseConn.getUrl_database()) {
					conflictAnalysis.conflict_Analysis_English(frame,
							"fileservices");
				} else {
					conflictAnalysis.conflict_Analysis_English(frame);
				}
			}
		}
		// ���ɱ���
		if (cmd.equals("���ɱ���") || cmd.equals("Generate Report")) {
			report(frame);
		}
		// ���������
		if (cmd.equals("���������") || cmd.equals("Analyze Result")) {
			result(frame);
		}
		// Ӣ��
		if (cmd.equals("Ӣ��") || cmd.equals("English")) {
			frame.updateMenuBarToEnglish();
		}
		// ����
		if (cmd.equals("����") || cmd.equals("Chinese")) {
			frame.updateMenuBarToChinese();
		}
		// ����
		if (cmd.equals("����") || cmd.equals("Help")) {
			help(frame);
		}
		// ����
		if (cmd.equals("����") || cmd.equals("About")) {
			about(frame);
		}
		// �˳�
		if (cmd.equals("�˳�(X)") || cmd.equals("Exit(X)")) {
			System.exit(0);
		}

	}

}

/**
 * �ļ���ȡ�߳�
 */
class FileReadThread implements Runnable {
	
	private Thread runner;
	private Service service;

	public FileReadThread(Service service) {
       this.service = service;
	}

	public void start() {
		// TODO Auto-generated method stub
		runner = new Thread(this);
		runner.start();
	}

	@Override
	public void run() {
		String analysisInfo = "Policy file is analyzing...";
		service.getFrame().getMessageArea().setText(analysisInfo + "\n");
		FileInputStream fis = null;
		InputStreamReader isr = null;
		// activity ��
		service.getPolicyDAO().addActivity("R", "FileServices");
		service.getPolicyDAO().addActivity("W", "FileServices");
		service.getPolicyDAO().addActivity("X", "FileServices");
		// measure��
		service.getPolicyDAO().addMeasure("permit", "deny");
		service.getPolicyDAO().addMeasure("deny", "permit");
		// semantic_activity��
		service.getPolicyDAO().addActivitySemantic("R", "Read");
		service.getPolicyDAO().addActivitySemantic("W", "Write");
		service.getPolicyDAO().addActivitySemantic("X", "Execute");
		// System.out.println("********************");
		// sematic_meature��
		service.getPolicyDAO().addMeasureSemantic("permint", "", 0);
		service.getPolicyDAO().addMeasureSemantic("deny", "", 0);
		// System.out.println("********************");
		BufferedReader br = null; // ���ڰ�װInputStreamReader,��ߴ������ܡ���ΪBufferedReader�л���ģ���InputStreamReaderû�С�
		try {
			// ʵ�������ݿ��л��Ļ�ÿ�����ݿ��еı�����Ѿ������õ�
			String str = "";
			// System.out.println("********************");
			// fis = new
			// FileInputStream("./resources/CNDP_FileService.txt");//
			// FileInputStream
			// fis = new
			// FileInputStream("K:/javatest/CNDPCD-20130314/bin/CNDP_FileService.txt");
			// akisn0w
			// fis = new
			// FileInputStream("C:/Users/Administrator/Desktop/VisualADS/policy/CNDP.txt");
			// fis = new
			// FileInputStream("C:/Users/Administrator/Desktop/VisualADS/CNDPCD/resources/CNDP_FileService.txt");
			// ���ļ�ϵͳ�е�ĳ���ļ��л�ȡ�ֽ�
			// ������ǵ�һ�ζ�ȡ��������ļ��򵥴������ݿ��еı���ռ���

			// �������·�� �жϸ�����Ǵ��ĸ����������е� JAR����eclipse
			String PFilePath = service.initPolicyDir() + "CNDP_FileService.txt";
			fis = new FileInputStream(PFilePath);

			isr = new InputStreamReader(fis);// InputStreamReader
			// ���ֽ���ͨ���ַ���������,
			br = new BufferedReader(isr);// ���ַ��������ж�ȡ�ļ��е�����,��װ��һ��new
			// InputStreamReader�Ķ���

			int authPFlag = 0;
			int inhePFlag = 0;

			Set<String> contextSet = new HashSet<String>();
			Set<String> viewSet = new HashSet<String>();
			Set<String> semantic_viewSet = new HashSet<String>();
			Set<String> semantic_contextSet = new HashSet<String>();

			Set<String> roleSet = new HashSet<String>();

			String domainName = null;

			int domainNum = 0;

			while ((str = br.readLine()) != null) {
				// ������{��������һ�������ز��Դ��� ������}�������������Ĵ���
				// �ؼ��� Authorization: Inheritance: ChildGroup: ChildUser:
				// NULL Everyone �ָ���::
				// ��������

				if (str.endsWith("{")) {
					domainName = str.substring(0, str.length() - 1);
					analysisInfo = "Starting parsing domain: " + domainName;
					service.getFrame().getMessageArea().append(analysisInfo + "\n");
					// frame.getMessageArea().setText(analysisInfo);
					// System.out.println(domainName);
				}
				if (str.equals("}")) {
					// һ������Ϣ�Ĵ������
					// ��һ���������������������
					analysisInfo = "Finished parsing domain: " + domainName;
					service.getFrame().getMessageArea().append(analysisInfo + "\n");
					// frame.getMessageArea().setText(analysisInfo);
					service.getDomainNames()[domainNum] = domainName;
					domainNum++;
					domainName = null;
					authPFlag = 0;
					inhePFlag = 0;
				}

				if (inhePFlag == 1) {
					// ���̳в��Բ��뵽��Ӧ�ı���
					
					String[] s = str.split(":");
					// role������Ԫ��
					String roleParentElement = s[0].substring(1, s[0]
							.indexOf("ChildGroup") - 1);

					if (roleSet.isEmpty()) {
						roleSet.add(domainName + "\\" + roleParentElement);
						service.getPolicyDAO().addRoleSemantic(domainName + "\\"
								+ roleParentElement, "Group");
					}
					if (!roleSet.contains(domainName + "\\"
							+ roleParentElement)) {
						roleSet.add(domainName + "\\" + roleParentElement);
						service.getPolicyDAO().addRoleSemantic(domainName + "\\"
								+ roleParentElement, "Group");
					}

					String roleElement = null;
					// semanticrole������Ԫ��
					// String role_RoleSemanticTable = null;
					// String semantic_RoleSemanticTable = null;
					String sChildGroup = s[1].substring(0, s[1]
							.indexOf("ChildUser"));
					String[] childGroup = sChildGroup.split(">");
					for (int i = 0; i < childGroup.length; i++) {
						System.out.println("*" + childGroup[i] + "*");

					}
					for (int i = 0; i < childGroup.length; i++) {
						if (!(childGroup[i].contains("?"))) {
							roleElement = childGroup[i].substring(1,
									childGroup[i].length());

							System.out.println("----" + roleElement);

							service.getPolicyDAO().addRole(roleElement, domainName
									+ "\\" + roleParentElement);
							if (roleSet.isEmpty()) {
								roleSet
										.add(domainName + "\\"
												+ roleElement);
								service.getPolicyDAO().addRoleSemantic(domainName
										+ "\\" + roleElement, "Group");
							}
							if (!roleSet.contains(domainName + "\\"
									+ roleElement)) {
								roleSet
										.add(domainName + "\\"
												+ roleElement);
								service.getPolicyDAO().addRoleSemantic(domainName
										+ "\\" + roleElement, "Group");
							}
						}
					}
					String[] childUser = s[2].split(">");
					for (int j = 0; j < childUser.length; j++) {
						if (!(childUser[j].contains("?"))) {
							roleElement = childUser[j];
							roleElement = childUser[j].substring(1,
									childUser[j].length());

							System.out.println("----" + roleElement);

							service.getPolicyDAO().addRole(roleElement, domainName
									+ "\\" + roleParentElement);
							if (roleSet.isEmpty()) {
								roleSet
										.add(domainName + "\\"
												+ roleElement);
								service.getPolicyDAO().addRoleSemantic(domainName
										+ "\\" + roleElement, "User");
							}
							if (!roleSet.contains(domainName + "\\"
									+ roleElement)) {
								roleSet
										.add(domainName + "\\"
												+ roleElement);
								service.getPolicyDAO().addRoleSemantic(domainName
										+ "\\" + roleElement, "User");
							}
						}
					}
				}

				if (str.equals("Inheritance:")) {
					authPFlag = 0;
					inhePFlag = 1;
					analysisInfo = "Parsing the inheritance policy...";
					service.getFrame().getMessageArea().append(analysisInfo + "\n");
					// System.out.println("��Ȩ����");
				}
				if (authPFlag == 1) {
					// ����Ȩ���Բ��뵽��Ӧ�ı���
					// tech.adtest.net\ly:KIRA\just for test3:RX:deny
					// s[0]���û��������� S[1]�û��� s[2]������ s[3]�ļ����� s[4]��� s[5]��ʩ
					String[] s = str.split(":|\\\\");
					// �г�ÿ�����Ե�ÿ��Ԫ������
					/*
					 * for(int i = 0; i< s.length; i++){
					 * System.out.println(s[i]); }
					 */
					// ���Ա�(policy)�Ĳ���
					CndPolicy cndPolicy = new CndPolicy();
					try {
						cndPolicy.setType(0);
						cndPolicy.setContext(s[2]);
						cndPolicy.setRole(s[0] + "\\" + s[1]);
						// System.out.println("S[0]=" + s[0] + "\n");
						cndPolicy.setView(s[3]);
						cndPolicy.setActivity(s[4]);
						cndPolicy.setMeasure(s[5]);
						cndPolicy.setOrganization(domainName);
						// System.out.println(domainName);

					} catch (NullPointerException ex) {
						ex.printStackTrace();
					}
					service.getPolicyDAO().addPolicy(cndPolicy);
					// context��Ĳ���
					if (contextSet.isEmpty()) {
						contextSet.add(domainName + "\\" + s[2]);
						service.getPolicyDAO().addContext(s[2], domainName);
						// System.out.println("*********view��***********"+s[2]);
					}
					if (!contextSet.contains(domainName + "\\" + s[2])) {
						service.getPolicyDAO().addContext(s[2], domainName);
						contextSet.add(domainName + "\\" + s[2]);
						// System.out.println("**********view��**********"+s[2]);
					}
					// view��Ĳ���
					if (viewSet.isEmpty()) {
						viewSet.add(domainName + "\\" + s[2] + "\\" + s[3]);
						service.getPolicyDAO().addView(s[3], domainName + "\\"
								+ s[2]);
						// System.out.println("*********view��***********"+s[2]);
					}
					if (!viewSet.contains(s[2] + "\\" + s[3])) {
						service.getPolicyDAO().addView(s[3], domainName + "\\"
								+ s[2]);
						viewSet.add(domainName + "\\" + s[2] + "\\" + s[3]);
						// System.out.println("**********view��**********"+s[2]);
					}
					// �����Ĳ���
					// semaitic_role�� �̳в���������

					// semantic_view�� �ļ�����������+�ļ���������ʶ��
					if (semantic_viewSet.isEmpty()) {
						semantic_viewSet.add(domainName + "\\" + s[2]
								+ "\\" + s[3]);
						service.getPolicyDAO().addViewSemantic(domainName + "\\"
								+ s[2] + "\\" + s[3], "File");
						// System.out.println("*********semantic_view��***********"+s[2]);
					}
					if (!semantic_viewSet.contains(domainName + "\\" + s[2]
							+ "\\" + s[3])) {
						service.getPolicyDAO().addViewSemantic(domainName + "\\"
								+ s[2] + "\\" + s[3], "File");
						semantic_viewSet.add(domainName + "\\" + s[2]
								+ "\\" + s[3]);
						// System.out.println("**********semantic_view��**********"+s[2]);
					}
					// sematic_context��
					if (semantic_contextSet.isEmpty()) {
						semantic_contextSet.add(domainName + "\\" + s[2]);
						service.getPolicyDAO().addContextSemantic(domainName + "\\"
								+ s[2], "host");
						// System.out.println("**********sematic_context��**********"+s[1]);
					}
					if (!semantic_contextSet.contains(domainName + "\\"
							+ s[2])) {
						service.getPolicyDAO().addContextSemantic(domainName + "\\"
								+ s[2], "host");
						semantic_contextSet.add(domainName + "\\" + s[2]);
						// System.out.println("**********sematic_context��**********"+s[1]);
					}
				}
				if (str.equals("Authorization:")) {
					authPFlag = 1;
					inhePFlag = 0;
					analysisInfo = "Parsing the authorization policy...";
					service.getFrame().getMessageArea().append(analysisInfo + "\n");
					// System.out.println("��Ȩ����");
				}
			}
		} catch (FileNotFoundException e) {
			System.out.println("�Ҳ���ָ���ļ�");
			JOptionPane.showMessageDialog(null, "File not Found Error",
					"Error", JOptionPane.ERROR_MESSAGE);
		} catch (IOException e) {
			System.out.println("��ȡ�ļ�ʧ��");
			JOptionPane.showMessageDialog(null, "File read Error",
					"Error", JOptionPane.ERROR_MESSAGE);
		} finally {
			try {
				br.close();
				isr.close();
				fis.close();
				// �رյ�ʱ����ð����Ⱥ�˳��ر���󿪵��ȹر������ȹ�s,�ٹ�n,����m
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		analysisInfo = "policy analysis success!";
		service.getFrame().getMessageArea().append(analysisInfo + "\n");

	}

}

