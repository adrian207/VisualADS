package cnd.conflict.frame;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import cnd.conflict.dao.PolicyDAO;


public class ResultAnalysisFrame extends JFrame{

	/**
	 * @param args
	 */
	private PolicyDAO policyDAO = new PolicyDAO();
	JLabel label = new JLabel();
	Frame frame;

	public ResultAnalysisFrame(Frame frame) { // ���캯��
		super("�������ͼ"); // ���ø��๹�캯��
		this.frame = frame;
		if (frame == null || frame.getLanguage() == 1) {
			setTitle("�������ͼ");
			init("�������ͼ");
		}
		else {
			setTitle("Result Analysis Diagram");
			init("Result Analysis Diagram");
		}
	}
	JScrollPane jScrollPane = new JScrollPane();
	public void init(String s) {
		Action[] actions = // Action����,���ֲ�������
		{ new ContextAction(s)};
		Container container = getContentPane(); // �õ�����
		container.add(createJToolBar(actions), BorderLayout.NORTH); // ���ӹ�����
		
		container.add(label, BorderLayout.CENTER); // �����ı�����
		setSize(780, 700); // ���ô��ڳߴ�
		setVisible(true); // ���ô��ڿ���
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE); // �رմ���ʱ�˳�����
	}

	private JToolBar createJToolBar(Action[] actions) { // ����������
		JToolBar toolBar = new JToolBar(); // ʵ����������
		for (int i = 0; i < actions.length; i++) {
			JButton bt = new JButton(actions[i]); // ʵ�����µİ�ť
			bt.setRequestFocusEnabled(false); // ���ò���Ҫ����
			toolBar.add(bt); // ���Ӱ�ť��������
		}
		return toolBar; // ���ع�����
	}

	class ContextAction extends AbstractAction {
		public ContextAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			 ImageIcon icon = new ImageIcon("img/result.jpg");
			 label.setIcon(icon);
			
		}
	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub

		new ResultAnalysisFrame(null);
	}

}
