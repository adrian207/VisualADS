package cnd.conflict.detect;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.border.Border;

import org.jgraph.JGraph;
import org.jgraph.graph.ConnectionSet;
import org.jgraph.graph.DefaultEdge;
import org.jgraph.graph.DefaultGraphCell;
import org.jgraph.graph.DefaultGraphModel;
import org.jgraph.graph.DefaultPort;
import org.jgraph.graph.GraphConstants;
import org.jgraph.graph.GraphModel;

import cnd.conflict.dao.PolicyDAO;
import cnd.conflict.frame.Frame;
import cnd.conflict.graph.Corner;
import cnd.conflict.graph.GenerateGraph;
import cnd.conflict.graph.Rule;
import cnd.conflict.test.ScrollablePicture;


/**
 * @author YC
 * 
 */
public class RelationGraph extends JFrame {
	private Frame frame;
	private Rule columnView;
	private Rule rowView;
	
	private static RelationGraph relationGraph = null;

	private PolicyDAO policyDAO = new PolicyDAO();
	JLabel label = new JLabel();

	private RelationGraph(String s) { // ���캯��
		super(s); // ���ø��๹�캯��
	}

	JScrollPane jScrollPane = new JScrollPane();

	public void init(Frame frame) {
		this.frame = frame;
		
		Container container = getContentPane(); // �õ�����
		container.removeAll();
		if (frame == null || frame.getLanguage() == 1) {
			relationGraph.setTitle("Ԫ���ϵͼ\n");
			Action[] actions = // Action����,���ֲ�������
				{ new ContextAction("�����Ĺ�ϵͼ"), new RoleAction("��ɫ��ϵͼ"), new ViewAction("��ͼ��ϵͼ"),
				  new ActivityAction("���ϵͼ"), new MeasureAction("�ֶι�ϵͼ"), new ExitAction("�˳�") };
			container.add(createJToolBar(actions), BorderLayout.NORTH); // ���ӹ�����
		}
		else {
			relationGraph.setTitle("Tuple Graph\n");
			Action[] actions = // Action����,���ֲ�������
				{ new ContextAction("Context Graph"), new RoleAction("Role Graph"),
				  new ViewAction("View Graph"), new ActivityAction("Activity Graph"),
				  new MeasureAction("Measure Graph"), new ExitAction("Close") };
			container.add(createJToolBar(actions), BorderLayout.NORTH); // ���ӹ�����
		}
		
		

		JPanel pLeft = new JPanel(), pRight = new JPanel(),
		// pTop = new JPanel(),
		pBottom = new JPanel();

		Border loweredbevel = BorderFactory.createLoweredBevelBorder();
		RelationGraph.this.jScrollPane.setViewportBorder(loweredbevel); 
		//����ͼ��ı��
        // Create the row and column headers.
        columnView = new Rule(Rule.HORIZONTAL, true);
        columnView.setPreferredWidth(400);
        rowView = new Rule(Rule.VERTICAL, true);
        rowView.setPreferredHeight(400);
        
        JToggleButton isMetric = new JToggleButton("cm", true);
        // Create the corners.
        JPanel butCorner = new JPanel();
        isMetric = new JToggleButton("cm", true);
        isMetric.setFont(new Font("SansSerif", Font.PLAIN, 11));
        isMetric.setMargin(new Insets(2,2,2,2));
        isMetric.addItemListener(new UnitsListener());
        butCorner.add(isMetric); //Use the default FlowLayout
                
        jScrollPane.setPreferredSize(new Dimension(300, 250));
    
        jScrollPane.setColumnHeaderView(columnView);	        
        jScrollPane.setRowHeaderView(rowView);
        jScrollPane.setCorner(JScrollPane.UPPER_LEFT_CORNER, 
                                    butCorner);
        jScrollPane.setCorner(JScrollPane.LOWER_LEFT_CORNER,
                                    new Corner());
        jScrollPane.setCorner(JScrollPane.UPPER_RIGHT_CORNER,
                                    new Corner());
				
		container.add(jScrollPane, BorderLayout.CENTER); // �����ı�����
	 
		container.add(pLeft, BorderLayout.WEST);
		container.add(pRight, BorderLayout.EAST);
		// container.add(pTop, BorderLayout.NORTH);
		container.add(pBottom, BorderLayout.SOUTH);

		setSize(500, 500); // ���ô��ڳߴ�
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
			
			JGraph graph = new GenerateGraph(frame).paintGraph("context");
	        RelationGraph.this.jScrollPane.setViewportView(graph);
						
		}
	}
	
    class UnitsListener implements ItemListener {
        public void itemStateChanged(ItemEvent e) {
            if (e.getStateChange() == ItemEvent.SELECTED) {
                // Turn it to metric.
                rowView.setIsMetric(true);
                columnView.setIsMetric(true);
            } else {
                // Turn it to inches.
                rowView.setIsMetric(false);
                columnView.setIsMetric(false);
            }
            //picture.setMaxUnitIncrement(rowView.getIncrement());
        }
    }

	class RoleAction extends AbstractAction {
		public RoleAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			/*
			 * ImageIcon icon = new ImageIcon("img/role.gif");
			 * label.setIcon(icon);
			 */

			JGraph graph = new GenerateGraph(frame).paintGraph("role");
			RelationGraph.this.jScrollPane.setViewportView(graph);
		}
	}

	class ViewAction extends AbstractAction {
		public ViewAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			/*
			 * ImageIcon icon = new ImageIcon("img/view.gif");
			 * label.setIcon(icon);
			 */
			JGraph graph = new GenerateGraph(frame).paintGraph("view");
			RelationGraph.this.jScrollPane.setViewportView(graph);
		}
	}

	class ExitAction extends AbstractAction {
		public ExitAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			// System.exit(0);
			RelationGraph.this.dispose();
		}
	}

	class ActivityAction extends AbstractAction {
		public ActivityAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			/*
			 * ImageIcon icon = new ImageIcon("img/activity.gif");
			 * label.setIcon(icon);
			 */
			JGraph graph = new GenerateGraph(frame).paintGraph("activity");
			RelationGraph.this.jScrollPane.setViewportView(graph);
		}
	}

	class MeasureAction extends AbstractAction {
		public MeasureAction(String s) {
			super(s);
		}

		public void actionPerformed(ActionEvent e) {
			/*
			 * ImageIcon icon = new ImageIcon("img/measure.gif");
			 * label.setIcon(icon);
			 */
		}
	}

	/**
	 * ��ȡ��̬ʵ��
	 * 
	 * @return RelationGraph
	 */
	public static RelationGraph getInstance(Frame frame) {
		if (relationGraph == null) {
			relationGraph = new RelationGraph("Ԫ���ϵͼ");
		}
		if (frame == null || frame.getLanguage() == 1)
			relationGraph.setTitle("Ԫ���ϵͼ\n");
		else
			relationGraph.setTitle("Tuple Graph\n");
		return relationGraph;
	}

	public static void main(String[] args) {
		new RelationGraph("Ԫ���ϵͼ");
	}
}
