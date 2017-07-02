package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class IDLC extends AbstractPage{
	
	public IDLC (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "frmMiniLogin")
	private WebElement loginBtn;
	@FindBy(xpath = "//div[@class='EncartInfos']//input[@name='ctl00$ctl00$cphMainContent$cphMainContent$txbMail']")
	private WebElement email;
	
	@FindBy(xpath = "//div[@class='EncartInfos']//input[@name='ctl00$ctl00$cphMainContent$cphMainContent$txbPassword']")
	private WebElement password;

	@FindBy(xpath = "//span[contains(text(),'CONNEXION')]")
	private WebElement submitLogin;
	

	@FindBy(xpath = "//span[contains(text(),'Se déconnecter')]")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//span[@class='lblNomPrenom']")
	private WebElement dashBoard;
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void submit(){

		submitLogin.click();
	}
	public void goTodDashboard()
	{		
		hover(dashBoard);	
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//span[text()='Bonjour mooltipass']"));
		return isElementPresent(By.xpath( "//span[text()='Bonjour mooltipass']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("frmMiniLogin"));
	}

}
