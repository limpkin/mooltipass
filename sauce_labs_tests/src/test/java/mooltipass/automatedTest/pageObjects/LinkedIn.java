package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class LinkedIn extends AbstractPage{
	
	public LinkedIn (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(id = "login-email")
	private WebElement email;
	
	@FindBy(id = "login-password")
	private WebElement password;

	@FindBy(id = "login-submit")
	private WebElement submitLogin;
	

	@FindBy(xpath = "//a[@data-control-name='nav.settings_signout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//img[@class='nav-item__profile-member-photo nav-item__icon ghost-person']")
	private WebElement dashBoard;
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	
	public void submit(){

		submitLogin.click();
	}
	public void goTodDashboard()
	{		
		dashBoard.click();	
	}
	public void logout(){		
		clickWithAction(logoutBtn);
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//img[@class='nav-item__profile-member-photo nav-item__icon ghost-person']"));
		return isElementPresent(By.xpath( "//img[@class='nav-item__profile-member-photo nav-item__icon ghost-person']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("login-email"));
	}


}
